#include "updater.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QCoreApplication>
#include <QApplication>
#include <QProgressDialog>

Updater::Updater(QObject* parent) : QObject(parent) {
    manager_ = new QNetworkAccessManager(this);
    local_version_ = QVersionNumber::fromString(VERSION);
}

void Updater::AutoCheck(){
    FetchManifest(false);
}

void Updater::ManualCheck(){
    FetchManifest(true);
}

void Updater::FetchManifest(bool manual){
    QNetworkRequest req((QUrl(Update::kManifestUrl)));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply* reply = manager_->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, manual](){
        const bool ok = (reply->error() == QNetworkReply::NoError);
        const QByteArray data = reply->readAll();
        reply->deleteLater();
        if(!ok){
            if(manual){
                QMessageBox::warning(nullptr, "Обновление", "Не удалось связаться с сервером обновлений.");
            }
            return;
        }
        OnManifest(data, manual);
    });
}

void Updater::OnManifest(const QByteArray& data, bool manual){
    const QJsonObject obj = QJsonDocument::fromJson(data).object();
    latest_version_ = obj.value("version").toString();
    download_url_ = obj.value("url").toString();
    const QString notes = obj.value("notes").toString();
    const QVersionNumber net = QVersionNumber::fromString(latest_version_);
    if(net.isNull() || download_url_.isEmpty()){
        if(manual){
            QMessageBox::warning(nullptr, "Обновление", "Некорректные данные о версии на сервере.");
        }
        return;
    }
    if(local_version_ < net){
        QString text = "Доступна новая версия " + latest_version_ + ".";
        if(!notes.isEmpty()){
            text += "\n\nЧто нового:\n" + notes;
        }
        text += "\n\nОбновить сейчас?";
        if(QMessageBox::question(nullptr, "Обновление", text, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){
            StartDownload(download_url_);
        }
    } else if(manual){
        QMessageBox::information(nullptr, "Обновление", "Установлена последняя версия " + QString(VERSION) + ".");
    }
}

void Updater::StartDownload(const QString& url){
    const QString path = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath("SmartView-update-setup.exe");
    QFile* file = new QFile(path);
    if(!file->open(QIODevice::WriteOnly)){
        QMessageBox::warning(nullptr, "Обновление", "Не удалось сохранить файл обновления.");
        delete file;
        return;
    }
    QNetworkRequest req((QUrl(url)));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply* reply = manager_->get(req);

    QProgressDialog* dlg = new QProgressDialog("Загрузка обновления…", "Отмена", 0, 100);
    dlg->setWindowTitle("Обновление");
    dlg->setWindowModality(Qt::ApplicationModal);
    dlg->setMinimumDuration(0);

    connect(reply, &QNetworkReply::readyRead, file, [reply, file](){
        file->write(reply->readAll());
    });
    connect(reply, &QNetworkReply::downloadProgress, dlg, [dlg](qint64 received, qint64 total){
        if(total > 0){
            dlg->setValue(static_cast<int>(received * 100 / total));
        }
    });
    connect(dlg, &QProgressDialog::canceled, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [this, reply, file, dlg, path](){
        file->write(reply->readAll());
        file->close();
        const bool ok = (reply->error() == QNetworkReply::NoError);
        reply->deleteLater();
        file->deleteLater();
        dlg->deleteLater();
        if(!ok){
            QFile::remove(path);
            QMessageBox::warning(nullptr, "Обновление", "Загрузка обновления не завершена.");
            return;
        }
        // Запустить установщик и закрыть приложение, чтобы он мог обновить файлы.
        if(QProcess::startDetached(path, QStringList())){
            qApp->exit(0);
        } else {
            QMessageBox::warning(nullptr, "Обновление", "Не удалось запустить установщик:\n" + path);
        }
    });
}
