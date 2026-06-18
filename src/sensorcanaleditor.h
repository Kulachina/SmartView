#ifndef SENSORCANALEDITOR_H
#define SENSORCANALEDITOR_H
#pragma once
#include <QWidget>
#include <QStringList>
#include "Data.h"

class DataBase;
class ChartView;
class QListWidget;

// Окно «Приборы и каналы»: выбор/настройка каналов прибора (цвет, единицы,
// тип погрешности, допуски) перед добавлением их на график.
class SensorCanalEditor : public QWidget {
    Q_OBJECT
public:
    SensorCanalEditor(DataBase& data_base,
                      ChartView* chart_view,
                      QVector<DataSeriesSensor>& data_sensor,
                      QVector<DataSeriesSensor>& all_data_sensor,
                      QWidget* parent = nullptr);
    void OpenForNew();    // показать только что загруженные приборы (data_sensor_)
    void OpenForEdit();   // редактировать уже добавленные приборы (all_data_sensor_)

private:
    void PopulateAndShow(QVector<DataSeriesSensor>& vec_data);
    void Clear();
    void CreatePanelLegend();
    void SetPanelLegend();
    void RebuildCanal(Canal& canal);
    void CheckCanals();
    void UncheckCanals();
    DataSeriesSensor* PreparationData();

    DataBase& data_base_;
    ChartView* chart_view_;
    QVector<DataSeriesSensor>& data_sensor_;
    QVector<DataSeriesSensor>& all_data_sensor_;
    QListWidget* sensor_list_;
    QListWidget* canal_list_;
    bool foo_call_ = false;
    QStringList chanels_ = {"GK-ГК","Q-Расход","Т-Температура","E-Термокомпенсация","Р-Давление","VLG-Влагомер","REZ-Резистивиметр","PL-Плотность флюида"};
};

#endif // SENSORCANALEDITOR_H
