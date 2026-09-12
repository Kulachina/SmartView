#ifndef PROTOCOLCATALOG_H
#define PROTOCOLCATALOG_H
#pragma once
#include <QMap>
#include <QString>
#include <QStringList>

// Списки выбираемых значений формы «Настройка протокола» — наименования
// аппаратуры, типы аппаратуры, заказчики, средства калибровки. В коде их
// держать нельзя: список пополняется кнопкой «+» прямо из формы и должен
// пережить обновление программы. Поэтому значения по умолчанию лежат в
// ресурсе :/protocol_lists.json, а рабочая копия — в файле
// protocol_lists.json каталога данных пользователя (его же можно править
// вручную в любом текстовом редакторе).
class ProtocolCatalog {
public:
    // Ключи разделов JSON.
    static const QString kDeviceNames;
    static const QString kDeviceTypes;
    static const QString kCustomers;
    static const QString kCalibMeans;

    enum class AddResult {
        Added,       // пункт добавлен и файл сохранён
        Empty,       // пустая строка — добавлять нечего
        Duplicate,   // такой пункт в разделе уже есть
        SaveFailed,  // пункт добавлен в память, но файл записать не удалось
    };

    ProtocolCatalog();

    QStringList Items(const QString& key) const { return lists_.value(key); }
    // Добавляет пункт в конец раздела и сразу сохраняет файл.
    AddResult Add(const QString& key, const QString& value);

    // Путь рабочей копии — для сообщений об ошибке записи.
    QString FilePath() const { return path_; }

private:
    void Load();
    bool Save() const;

    QString path_;
    QMap<QString, QStringList> lists_;
};

#endif // PROTOCOLCATALOG_H
