#include "updater.h"
#include "QMessageBox"

Updater::Updater() {}

void Updater::CheckVersion(){
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QUrl url("https://kulachina.github.io/SmartView/update/Updates.xml");
    connect(manager, &QNetworkAccessManager::finished, this,[=](QNetworkReply *reply){
        if(reply->error() != QNetworkReply::NoError){
            QMessageBox::warning(nullptr,"Ошибка", reply->errorString());
            reply->deleteLater();
            return;
        }
        QByteArray data = reply->readAll();
        QXmlStreamReader xml(data);
        QString latest_version;
        while(!xml.atEnd()){
            xml.readNext();
            if(xml.readNextStartElement() && xml.name() == "Version"){
                latest_version = xml.readElementText();
                break;
            }
        }
        if(xml.hasError()){
            QMessageBox::warning(nullptr,"Ошибка разбора XML:", xml.errorString());
            reply->deleteLater();
            return;
        }
        QVersionNumber v1 = QVersionNumber::fromString(latest_version);
        QVersionNumber v2 = QVersionNumber::fromString(VERSION);
        if (v2 < v1){
            int res = QMessageBox::question(nullptr,"Обновление", "Доступна новая версия для скачивания "  + latest_version + "/nУстановить сейчас?",QMessageBox::Yes,QMessageBox::No);
            if(res == QMessageBox::Yes){
                CheckUpdate();
            }
        } else {
            QMessageBox::information(nullptr,"Обновление","Установлена последняя версия " + latest_version);
        }
        reply->deleteLater();
    });
    manager->get(QNetworkRequest(url));
}
void Updater::CheckUpdate(){
    QString updater = QCoreApplication::applicationDirPath() + "/SmartViewUpdater.exe";
    if(!QFileInfo::exists(updater)){
        return;
    }
    QProcess::startDetached(updater,QStringList() << "--updater");
    qApp->quit();

}
void Updater::SetFlagExit(bool& flag){
    if(flag){
        flag = false;
    } else {
        flag = true;
    }
}

