#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QVersionNumber>
#include <QString>

#define VERSION "0.9.4"

namespace Update {
// Манифест версии в Object Storage (публичный объект). Заменишь на свой бакет.
inline const QString kManifestUrl = QStringLiteral("https://storage.yandexcloud.net/smartview-updates/version.json");
}

// Автообновление: читает version.json с сервера, сравнивает версию и, если новее,
// скачивает установщик и запускает его.
class Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(QObject* parent = nullptr);
    void AutoCheck();     // тихая проверка при старте (молчит, если обновлений нет)
    void ManualCheck();   // ручная проверка (сообщает и когда версия уже последняя)

private:
    void FetchManifest(bool manual);
    void OnManifest(const QByteArray& data, bool manual);
    void StartDownload(const QString& url);
    QNetworkAccessManager* manager_;
    QVersionNumber local_version_;
    QString latest_version_;
    QString download_url_;
};

#endif // UPDATER_H
