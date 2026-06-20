#include "licensemanager.h"
extern "C" {
#include "tweetnacl.h"
}
#include <QSysInfo>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>

// Публичный ключ Ed25519 (32 байта) — пара к приватному ключу сервера.
static const unsigned char kPublicKey[32] = {
    0xb5,0x82,0x49,0xb1,0xc4,0xfe,0x3e,0x13,0x51,0x10,0x72,0x67,0x3b,0xa2,0x64,0x71,
    0x69,0xcc,0x85,0x4f,0x26,0x36,0xe5,0xd4,0xd1,0xa3,0xf7,0x38,0x44,0x4f,0x06,0x58
};

// Проверяет подпись токена встроенным публичным ключом и возвращает payload (JSON).
// Пустой объект = подпись недействительна (токен подделан или повреждён).
static QJsonObject VerifyAndParse(const QString& token){
    const int dot = token.indexOf('.');
    if(dot <= 0){
        return QJsonObject();
    }
    const QByteArray payload = QByteArray::fromBase64(token.left(dot).toUtf8(), QByteArray::Base64UrlEncoding);
    const QByteArray sig = QByteArray::fromBase64(token.mid(dot + 1).toUtf8(), QByteArray::Base64UrlEncoding);
    if(sig.size() != 64 || payload.isEmpty()){
        return QJsonObject();
    }
    QByteArray sm = sig + payload;                 // crypto_sign_open ожидает подпись||сообщение
    QByteArray m(sm.size(), Qt::Uninitialized);
    unsigned long long mlen = 0;
    if(crypto_sign_open(reinterpret_cast<unsigned char*>(m.data()), &mlen,
                        reinterpret_cast<const unsigned char*>(sm.constData()),
                        static_cast<unsigned long long>(sm.size()), kPublicKey) != 0){
        return QJsonObject();   // подпись не сошлась
    }
    return QJsonDocument::fromJson(payload).object();
}

LicenseManager::LicenseManager(QObject* parent) : QObject(parent) {}

QString LicenseManager::MachineFingerprint(){
    QByteArray id = QSysInfo::machineUniqueId();
    if(id.isEmpty()){
        id = QSysInfo::machineHostName().toUtf8();   // запасной вариант
    }
    return QString::fromLatin1(QCryptographicHash::hash(id, QCryptographicHash::Sha256).toHex());
}

QString LicenseManager::TokenPath() const {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir + "/license.dat";
}

bool LicenseManager::SaveLicense(const QString& token, const QString& fingerprint, const QString& expires, const QString& organization){
    const QString path = TokenPath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QJsonObject obj;
    obj["token"] = token;
    obj["fingerprint"] = fingerprint;
    obj["expires"] = expires;
    obj["organization"] = organization;
    obj["last_validated"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    obj["max_seen"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    QFile f(path);
    if(!f.open(QIODevice::WriteOnly)){
        return false;
    }
    f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    f.close();
    return true;
}

QString LicenseManager::Organization() const {
    QFile f(TokenPath());
    if(!f.open(QIODevice::ReadOnly)){
        return QString();
    }
    const QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
    f.close();
    return obj.value("organization").toString();
}

LicenseManager::Status LicenseManager::CheckAtStartup(){
    QFile f(TokenPath());
    if(!f.open(QIODevice::ReadOnly)){
        return Status::NeedsActivation;
    }
    const QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
    f.close();
    const QString token = obj.value("token").toString();
    if(token.isEmpty()){
        return Status::NeedsActivation;
    }
    // Офлайн-проверка подписи встроенным публичным ключом: подделать локальный файл нельзя.
    const QJsonObject payload = VerifyAndParse(token);
    if(payload.isEmpty()){
        return Status::NeedsActivation;   // подпись недействительна
    }
    // Привязка к машине (из подписанного payload).
    if(payload.value("fp").toString() != MachineFingerprint()){
        return Status::NeedsActivation;
    }
    const QDateTime now = QDateTime::currentDateTimeUtc();
    // Срок действия (из подписанного payload).
    const qint64 exp = payload.value("exp").toInteger();
    if(exp > 0 && QDateTime::fromSecsSinceEpoch(exp, Qt::UTC) < now){
        return Status::Expired;
    }
    // Анти-откат часов: системное время раньше максимально виденного => часы перевели назад.
    const QDateTime max_seen = QDateTime::fromString(obj.value("max_seen").toString(), Qt::ISODate);
    const bool rolled_back = max_seen.isValid() && now.addSecs(License::kRollbackToleranceSecs) < max_seen;
    // Онлайн-проверка (heartbeat): сервер проверяет подпись токена, статус ключа, отзыв и срок.
    const NetCheck nc = ValidateOnline(token);
    if(nc == NetCheck::Invalid){
        return Status::NeedsActivation;   // ключ отозван или токен недействителен
    }
    if(nc == NetCheck::Valid){
        TouchLastValidated();   // подтверждено сервером — и двигаем max_seen вперёд
        return Status::Valid;
    }
    // Сети нет.
    if(rolled_back){
        return Status::Error;   // часы отмотаны назад, подтвердить онлайн нельзя — не доверяем
    }
    // Разрешаем работать в пределах офлайн-грейса.
    const QDateTime last = QDateTime::fromString(obj.value("last_validated").toString(), Qt::ISODate);
    if(last.isValid() && last.secsTo(now) < qint64(License::kGraceDays) * 86400){
        RatchetMaxSeen();
        return Status::Valid;
    }
    return Status::Error;   // давно не было связи — нужно выйти в сеть
}

LicenseManager::NetCheck LicenseManager::ValidateOnline(const QString& token){
    if(License::kServerUrl.isEmpty()){
        return NetCheck::Offline;
    }
    QJsonObject body;
    body["token"] = token;
    body["fingerprint"] = MachineFingerprint();

    QNetworkAccessManager mgr;
    QNetworkRequest req((QUrl(License::kServerUrl + "/validate")));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = mgr.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(License::kNetTimeoutMs);
    loop.exec();
    if(!timer.isActive()){      // сработал таймаут — обрываем зависший запрос
        reply->abort();
    }
    timer.stop();

    if(reply->error() != QNetworkReply::NoError){
        reply->deleteLater();
        return NetCheck::Offline;   // нет связи / таймаут — решит грейс
    }
    const QJsonObject resp = QJsonDocument::fromJson(reply->readAll()).object();
    reply->deleteLater();
    return resp.value("valid").toBool() ? NetCheck::Valid : NetCheck::Invalid;
}

void LicenseManager::TouchLastValidated(){
    QFile f(TokenPath());
    if(!f.open(QIODevice::ReadOnly)){
        return;
    }
    QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
    f.close();
    const QDateTime now = QDateTime::currentDateTimeUtc();
    obj["last_validated"] = now.toString(Qt::ISODate);
    const QDateTime stored = QDateTime::fromString(obj.value("max_seen").toString(), Qt::ISODate);
    if(!stored.isValid() || now > stored){
        obj["max_seen"] = now.toString(Qt::ISODate);
    }
    if(f.open(QIODevice::WriteOnly)){
        f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        f.close();
    }
}

// Сдвигает сохранённое «максимально виденное время» вперёд (никогда не уменьшает).
void LicenseManager::RatchetMaxSeen(){
    QFile f(TokenPath());
    if(!f.open(QIODevice::ReadOnly)){
        return;
    }
    QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
    f.close();
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QDateTime stored = QDateTime::fromString(obj.value("max_seen").toString(), Qt::ISODate);
    if(stored.isValid() && now <= stored){
        return;   // часы не продвинулись — ничего не пишем
    }
    obj["max_seen"] = now.toString(Qt::ISODate);
    if(f.open(QIODevice::WriteOnly)){
        f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        f.close();
    }
}

LicenseManager::Result LicenseManager::Activate(const QString& key, const QString& organization){
    Result r;
    if(License::kServerUrl.isEmpty()){
        r.message = "Сервер активации ещё не настроен (URL не задан).";
        return r;
    }
    QJsonObject body;
    body["key"] = key.trimmed();
    body["fingerprint"] = MachineFingerprint();
    body["organization"] = organization.trimmed();
    body["version"] = License::kAppVersion;

    QNetworkAccessManager mgr;
    QNetworkRequest req((QUrl(License::kServerUrl + "/activate")));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = mgr.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(License::kNetTimeoutMs);
    loop.exec();
    if(!timer.isActive()){      // сработал таймаут — обрываем зависший запрос
        reply->abort();
    }
    timer.stop();

    if(reply->error() != QNetworkReply::NoError){
        r.message = "Сеть/сервер недоступны: " + reply->errorString();
        reply->deleteLater();
        return r;
    }
    const QJsonObject resp = QJsonDocument::fromJson(reply->readAll()).object();
    reply->deleteLater();
    const QString token = resp.value("token").toString();
    if(token.isEmpty()){
        r.message = resp.value("error").toString("Активация отклонена сервером.");
        return r;
    }
    if(!SaveLicense(token, MachineFingerprint(), resp.value("expires").toString(), organization.trimmed())){
        r.message = "Не удалось сохранить лицензию на диск.";
        return r;
    }
    r.ok = true;
    return r;
}
