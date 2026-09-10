#ifndef PROTOCOLWRITER_H
#define PROTOCOLWRITER_H
#pragma once
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QList>
#include <QVector>
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
#include "data_base.h"

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
    DataBase &data_base_;
    QComboBox* select_pribor_;
    QComboBox* select_name_type_;
    QComboBox* type_pribor_;
    QLineEdit* series_number_;
    // Средства калибровки: по чекбоксу на каждый пункт instruments_ —
    // в протокол попадают все отмеченные (в бланке под них 4 строки).
    QVector<QCheckBox*> instr_boxes_;
    QComboBox* list_client_;
    QCheckBox *canal_temp_;
    QCheckBox *canal_bar_;
    QList<QString> pribors_;
    QMap<QString,QString> map_pribors_;
    QWidget *bar_settings_widget_;
    QSpinBox *bar_points_spin_;
    QSpinBox *bar_temp_points_spin_;
    QWidget *temp_settings_widget_;
    QSpinBox *temp_points_spin_;
    QList<QString> instruments_ = {"Лабораторный электронный термометр ЛТ-300 зав.№ 898508",
                                   "Манометр цифровой ДМ5002М-Г зав.№ 0072",
                                   "Жидкостный термостат",
                                   "Ручной пресс для подачи высокого давления"};
    QList<QString> clients_ = {"ООО «РАИФ», ИНН 1658105798",
                               "ООО «Рэд Энерджи», ИНН 7720450334",
                               "ООО «НСК Барс», ИНН 1656093875",
                               "ООО «ТОГИС», ИНН 8603095617",
                               "ООО «НПК ПРОМСЕРВИС», ИНН 7724317703",
                               "ООО «КОМПЛЕКС», ИНН 6659127987",
                               "ООО «СГК», ИНН 8602217069",
                               "ООО «КВС ИНТЕРНЭШНЛ», ИНН 7725778471",
                               "ООО «ЮГС», ИНН 8612012254",
                               "ООО «НТП ВУГЭЦ», ИНН 1642003529",
                               "ООО «ПК ЧМВ», ИНН 9723195799",
                               "АО «СП МеКаМинефть», ИНН 8620006279",
                               "ИП «Русанов Н.Г.», ИНН 230606347220",
                               "ТОО «ГеоМунайРесурс», ИНН"};
    QList<QString> type_pribors_ = {"АЦМ-10",
                               "АЦМ-6",
                               "АЦМ-6Г",
                               "АЦМ-6УИРТ",
                               "АЦМ-8",
                               "АЦМ-8М",
                               "АЦМ-4М",
                               "АЦМ-6УИТ",
                               "АЦМ-6У",
                               "АЦМ-6УИ",
                               "АЦМ-8С",
                               "АЦМ-10С"};

};

#endif // PROTOCOLWRITER_H
