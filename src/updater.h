#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QXmlStreamReader>
#include <QVersionNumber>
#include <QFileInfo>
#include <QProcess>
#include <QCoreApplication>
#define VERSION "0.9.0"

class Updater : public QObject
{
    Q_OBJECT
public:
    Updater();
    void AutoCheck();
    void ManualCheck();
private:
    void CheckVersion();
    void CheckUpdate();
    QVersionNumber local_version_;
    QVersionNumber net_version_;
    QString latest_version_;
    QNetworkAccessManager *manager_;
    bool error_connect_ = false;
};

#endif // UPDATER_H
