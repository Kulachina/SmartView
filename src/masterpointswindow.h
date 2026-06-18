#ifndef MASTERPOINTSWINDOW_H
#define MASTERPOINTSWINDOW_H
#pragma once
#include <QWidget>
#include <QStringList>
#include "Data.h"

class DataBase;
class CreateRaport;
class QTabWidget;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QStandardItem;
class QStandardItemModel;

// Окно «Мастер точек»: таблицы интерполяции для GST/LAS и генерация отчётов.
class MasterPointsWindow : public QWidget {
    Q_OBJECT
public:
    MasterPointsWindow(DataBase& data_base, CreateRaport& create_raport, QWidget* parent = nullptr);
    void Open();   // перестроить содержимое таблиц и показать окно

private:
    QWidget* CreateTabMasterLAS();
    QWidget* CreateTabMasterACM();
    void CreateInContentLAS();
    void CreateInContentACM();
    void FilingTableLAS(QStandardItemModel* model);
    void FilingTable(QStandardItemModel* model_temp, QStandardItemModel* model_bar);
    void FillFromOneTable();
    void FillAllTables();
    void FillAllTablesLAS();
    void AnalisingSeries(const DataSeriesSensor& data);
    void AnalisingSeriesLAS(const DataSeriesSensor& data);
    void ChangeAllTables(QStandardItem* item);
    void CreateAllDoc();
    void CreateAllDocLAS();
    DataSeriesSensor* FindSensorData(const QString& name);

    DataBase& data_base_;
    CreateRaport& create_raport_;
    QSpinBox *s_et_bar_,
             *s_et_temp_,
             *s_et_bar_las_,
             *s_et_temp_las_;
    QDoubleSpinBox *s_step_bar_,
                   *s_step_bar_las_;
    QTabWidget *tab_,
               *tab_las_;
    QCheckBox *check_step_bar_,
              *check_step_bar_las_,
              *ch_sel_1_table_,
              *ch_sel_1_table_las_;
    QString name_canal_1_,
            name_canal_2_,
            name_canal_las_1_,
            name_canal_las_2_;
    QStringList chanels_ = {"GK-ГК","Q-Расход","Т-Температура","E-Термокомпенсация","Р-Давление","VLG-Влагомер","REZ-Резистивиметр","PL-Плотность флюида"};
    bool change_all_tables_ = false;
    bool flag_termocompens_ = false;
};

#endif // MASTERPOINTSWINDOW_H
