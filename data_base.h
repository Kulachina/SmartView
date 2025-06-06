#ifndef DATA_BASE_H
#define DATA_BASE_H
#include "Data.h"

#include <QMap>
#include <QPointer>

class DataBase {
public:
    DataBase();
    void AddDataSerEtalon(DataSeriesEtalon data);
    void AddDataSerACM(DataSeriesACM& data);
    void AddLabelSensor(QLabel* sensor,QLabel* name);
    void AddListAxis(QValueAxis *axis);
    void AddCheckPoint(qint64 q,double temp, int bar);
    const QVector<QDateTime>& GetCheckPoints();
    QVector<double>& GetCheckPointTemp();
    QVector<int>& GetCheckPointBar();
    QVector<DataSeriesACM>& GetDataSerACM();
    QVector<DataSeriesEtalon>& GetDataSerEtalon();
    QList<QPointer<QValueAxis>>& GetListAxis();
    void SetDefaultAxisX(QDateTime max, QDateTime min);
    std::pair<QDateTime,QDateTime> GetDefaultAxisX();
private:
    QVector<DataSeriesACM> data_acm_;
    QVector<DataSeriesEtalon> data_etalon_;
    QVector<QDateTime> check_points_;
    QVector<int> check_point_bar;
    QVector<double> check_points_temp;
    QList<QPointer<QLabel>> vector_name_sensor_;
    QList<QPointer<QLabel>> vector_name_;
    QList<QPointer<QValueAxis>> list_axis_y_;
    QMap<QString,DataSeriesACM*> map_data_sensor_;
    QDateTime axis_min_,
           axis_max_;
};

#endif // DATA_BASE_H
