#include "updater.h"
#include <QMessageBox>

Updater::Updater() {
    manager_ = new QNetworkAccessManager(this);
    connect(manager_, &QNetworkAccessManager::finished, this,[this](QNetworkReply *reply){
        if(reply->error() != QNetworkReply::NoError){
            QMessageBox::warning(nullptr,"Ошибка", reply->errorString());
            reply->deleteLater();
            return;
        }
        QByteArray data = reply->readAll();
        QXmlStreamReader xml(data);
        while(!xml.atEnd()){
            xml.readNext();
            if(xml.readNextStartElement() && xml.name() == "Version"){
                latest_version_ = xml.readElementText();
                break;
            }
        }
        if(xml.hasError()){
            QMessageBox::warning(nullptr,"Ошибка разбора XML:", xml.errorString());
            reply->deleteLater();
            return;
        }
        net_version_ = QVersionNumber::fromString(latest_version_);
        local_version_ = QVersionNumber::fromString(VERSION);
        if (local_version_ < net_version_){
            int res = QMessageBox::question(nullptr,"Обновление", "Доступна новая версия для скачивания "  + latest_version_ + "\n Установить сейчас?",QMessageBox::Yes,QMessageBox::No);
            if(res == QMessageBox::Yes){
                CheckUpdate();
            }
        }
        reply->deleteLater();
    });
}
void Updater::CheckVersion(){
    QUrl url("https://kulachina.github.io/SmartView/update/Updates.xml");
    manager_->get(QNetworkRequest(url));
}

void Updater::CheckUpdate(){
    QString updater = QCoreApplication::applicationDirPath() + "/SmartViewUpdater.exe";
    if(!QFileInfo::exists(updater)){
        return;
    }
    QProcess::startDetached(updater,QStringList() << "--updater");
    qApp->exit(0);
}
void Updater::AutoCheck(){
    CheckVersion();
}
void Updater::ManualCheck(){
    //CheckVersion();
    if (local_version_ < net_version_){
        int res = QMessageBox::question(nullptr,"Обновление", "Доступна новая версия для скачивания "  + latest_version_ + "\n Установить сейчас?",QMessageBox::Yes,QMessageBox::No);
        if(res == QMessageBox::Yes){
            CheckUpdate();
        }
    } else {
        QMessageBox::information(nullptr,"Обновление","Установлена последняя версия " + latest_version_);
    }
}

