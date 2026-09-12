#ifndef PROTOCOLWRITER_H
#define PROTOCOLWRITER_H
#pragma once
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QList>
#include <QVector>
#include <QLabel>
#include <QSpinBox>
#include <QVariant>
// Запись протокола калибровки в .xlsx на основе встроенного шаблона
// (:/protocol_template.xlsx) через QXlsx.
//
// Шаблон устроен так: лист «Сертификат» содержит ТОЛЬКО фиксированную шапку
// (строки 1..60 — логотип, метаданные, условия, средства калибровки), а
// результатная часть бланка лежит на скрытом листе «_proto» и служит донором
// стилей. Всё, что ниже строки 60, класс верстает сам, ведя курсор строки:
// число блоков давления, число точек и набор каналов переменные, а QXlsx не
// умеет вставлять/удалять строки — поэтому статической разметки там быть не может.
#include "protocoldata.h"
#include "protocolcatalog.h"
#include "data_base.h"

class QVBoxLayout;

class ProtocolWriter: public QWidget {
    Q_OBJECT
public:
    ProtocolWriter(DataBase& data_base,QWidget* parent = nullptr);
    void GetSpisokPribors();

    // Грузит шаблон, верстает протокол по data и сохраняет. parent — родитель
    // диалогов; save_path задаёт путь напрямую (пустой — спросить у пользователя).
    // Возвращает true при успешном сохранении, false при ошибке или отмене.
    bool Generate(const Protocol::Data& data, QWidget* parent = nullptr,
                  const QString& save_path = QString());

    // Собирает Protocol::Data по форме и данным выбранного прибора: настройки
    // каналов («Приборы и каналы»), эталон из контрольных точек и показания
    // прибора в тех же точках. Клетки, на которые КТ не хватило, остаются
    // пустыми — протокол печатается как бланк под ручное заполнение.
    Protocol::Data CollectData() const;
private:
    QString TrimNameandNumber(QString name);
    QWidget* WithAddButton(QComboBox* box, const QString& key, const QString& title);
    void AddInstrument();
    bool StoreNewItem(const QString& key, const QString& value);
    ProtocolCatalog catalog_;
    DataBase &data_base_;
    QComboBox* select_pribor_;
    QComboBox* select_name_type_;
    QComboBox* type_pribor_;
    QLineEdit* series_number_;
    // Средства калибровки: по чекбоксу на каждый пункт списка — в протокол
    // попадают все отмеченные (в бланке под них 4 строки).
    QVector<QCheckBox*> instr_boxes_;
    QVBoxLayout* instr_layout_;
    QComboBox* list_client_;
    QCheckBox *canal_temp_;
    QCheckBox *canal_bar_;
    QList<QString> pribors_;
    QMap<QString,QString> map_pribors_;
    QSpinBox *bar_points_spin_;
    QSpinBox *bar_temp_points_spin_;
    QSpinBox *temp_points_spin_;
    // Подписи счётчиков скрываются вместе со своими полями, когда канал снят.
    QLabel *temp_points_label_;
    QLabel *bar_points_label_;
    QLabel *bar_temp_points_label_;
};

#endif // PROTOCOLWRITER_H
