#include "protocolcatalog.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardPaths>

const QString ProtocolCatalog::kDeviceNames = QStringLiteral("device_names");
const QString ProtocolCatalog::kDeviceTypes = QStringLiteral("device_types");
const QString ProtocolCatalog::kCustomers   = QStringLiteral("customers");
const QString ProtocolCatalog::kCalibMeans  = QStringLiteral("calibration_means");

namespace {
const QString kFileName = QStringLiteral("protocol_lists.json");
const QString kDefaults = QStringLiteral(":/protocol_lists.json");

QStringList AllKeys() {
    return {ProtocolCatalog::kDeviceNames, ProtocolCatalog::kDeviceTypes,
            ProtocolCatalog::kCustomers,   ProtocolCatalog::kCalibMeans};
}
}   // namespace

ProtocolCatalog::ProtocolCatalog() {
    Load();
}

void ProtocolCatalog::Load() {
    path_ = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                .filePath(kFileName);

    // Рабочая копия пользователя, а если её ещё нет (или она пуста/битая) —
    // список по умолчанию из ресурсов.
    QByteArray raw;
    QFile user(path_);
    if (user.open(QIODevice::ReadOnly))
        raw = user.readAll();
    QJsonObject root = QJsonDocument::fromJson(raw).object();
    if (root.isEmpty()) {
        QFile defaults(kDefaults);
        if (defaults.open(QIODevice::ReadOnly))
            root = QJsonDocument::fromJson(defaults.readAll()).object();
    }

    for (const QString& key : AllKeys()) {
        QStringList items;
        const QJsonArray arr = root.value(key).toArray();
        for (const QJsonValue& value : arr) {
            const QString item = value.toString().trimmed();
            if (!item.isEmpty() && !items.contains(item))
                items.push_back(item);
        }
        lists_.insert(key, items);
    }
}

bool ProtocolCatalog::Save() const {
    QJsonObject root;
    for (auto it = lists_.cbegin(); it != lists_.cend(); ++it)
        root.insert(it.key(), QJsonArray::fromStringList(it.value()));

    if (!QDir().mkpath(QFileInfo(path_).absolutePath()))
        return false;
    QFile file(path_);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    return file.write(QJsonDocument(root).toJson(QJsonDocument::Indented)) != -1;
}

ProtocolCatalog::AddResult ProtocolCatalog::Add(const QString& key, const QString& value) {
    const QString item = value.trimmed();
    if (item.isEmpty())
        return AddResult::Empty;
    QStringList& items = lists_[key];
    if (items.contains(item))
        return AddResult::Duplicate;
    items.push_back(item);
    // Пункт остаётся доступным в текущем сеансе, даже если файл не записался.
    return Save() ? AddResult::Added : AddResult::SaveFailed;
}
