#ifndef LICENSEMANAGER_H
#define LICENSEMANAGER_H
#pragma once
#include <QObject>
#include <QString>

namespace License {
// TODO (Фаза 1): после публикации Cloud Function + API Gateway в Яндекс Облаке
// вставить базовый URL, например "https://<id>.apigw.yandexcloud.net".
inline const QString kServerUrl = QStringLiteral("https://d5doosdmu7nal65sht86.tmjd4m4j.apigw.yandexcloud.net");
// Принудительная проверка лицензии. Включить (true) после того, как сервер заработает.
inline constexpr bool kEnforce = true;
inline const QString kAppVersion = QStringLiteral("1.0");
// Сколько дней разрешено работать офлайн без успешной онлайн-проверки.
inline constexpr int kGraceDays = 7;
}

// Клиентская часть лицензирования: отпечаток машины, онлайн-активация ключа,
// хранение и офлайн-проверка токена. Сервер — источник истины.
class LicenseManager : public QObject {
    Q_OBJECT
public:
    enum class Status { Valid, NeedsActivation, Expired, Error };
    struct Result { bool ok = false; QString message; };

    explicit LicenseManager(QObject* parent = nullptr);
    // Стабильный идентификатор машины (SHA-256 от QSysInfo::machineUniqueId()).
    static QString MachineFingerprint();
    // Офлайн-проверка сохранённого токена при старте.
    Status CheckAtStartup();
    // Онлайн-активация ключа на сервере (блокирующая, для модального диалога).
    Result Activate(const QString& key);

private:
    enum class NetCheck { Valid, Invalid, Offline };
    QString TokenPath() const;
    bool SaveLicense(const QString& token, const QString& fingerprint, const QString& expires);
    NetCheck ValidateOnline(const QString& token);   // heartbeat /validate
    void TouchLastValidated();                        // обновить отметку успешной проверки
};

#endif // LICENSEMANAGER_H
