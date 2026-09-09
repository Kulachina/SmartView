#ifndef DATA_BASE_H
#define DATA_BASE_H
#pragma once
#include "Data.h"
#include <QMap>
#include <QPointer>

class DataBase {
public:
    DataBase();
    void AddDataSerEtalon(DataSeriesEtalon data);
    void AddDataSerACM(DataSeriesSensor data);
    void AddLabelSensor(QLabel* sensor,QLabel* name);
    void AddListAxis(QValueAxis *axis);
    void AddCheckPoint(qint64 q,double temp, double bar);
    void AddDeltaVolData(QString name_sensor, QVector<double> delta_bar, QVector<double> volume_bar,QVector<double> delta_temp, QVector<double> volume_temp);
    QVector<QDateTime>& GetCheckPoints();
    QVector<qint64>& GetCheckPoints64();
    QVector<double>& GetCheckPointTemp();
    QVector<double>& GetCheckPointBar();
    QVector<DataSeriesSensor>& GetDataSerACM();
    QVector<DataSeriesEtalon>& GetDataSerEtalon();
    QVector<CheckRange>& GetCheckRanges();
    // Условия проведения калибровки из файла эталона: [температура окружающей
    // среды °C, относительная влажность %, атмосферное давление мм.рт.ст].
    // Пустой вектор — в файле условий не было (старые .sml2).
    QVector<double>& GetConditions();
    QList<QPointer<QValueAxis>>& GetListAxis();
    void SetDefaultAxisX(QDateTime max, QDateTime min);
    std::pair<QDateTime,QDateTime> GetDefaultAxisX();
    void ClearAll();
    void CreatePointsDate();
    bool FindSensor(QString& name);
    bool FindCanal(const QString& sensor, const QString& canal);
private:
    QVector<DataSeriesSensor> data_acm_;
    QVector<DataSeriesEtalon> data_etalon_;
    QVector<CheckRange> check_ranges_;
    QVector<QDateTime> check_points_;
    QVector<qint64> check_points64_;
    QVector<double> check_point_bar;
    QVector<double> check_points_temp;
    QVector<double> conditions_;
    QList<QPointer<QLabel>> vector_name_sensor_;
    QList<QPointer<QLabel>> vector_name_;
    QList<QPointer<QValueAxis>> list_axis_y_;
    QMap<QString,DataSeriesSensor*> map_data_sensor_;
    QDateTime axis_min_,
           axis_max_;
};

#endif // DATA_BASE_H
