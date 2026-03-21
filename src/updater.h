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
#define VERSION "0.8.6"

class Updater : public QObject
{
    Q_OBJECT
public:
    Updater();
    void CheckVersion();
    void SetFlagExit(bool& flag);
private:
    void CheckUpdate();
};

#endif // UPDATER_H
