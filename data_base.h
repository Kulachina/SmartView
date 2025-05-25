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
    void AddCheckPoint(qreal q);
    const QVector<qreal>& GetCheckPoints();
    QVector<DataSeriesACM>& GetDataSerACM();
    QVector<DataSeriesEtalon>& GetDataSerEtalon();
    QList<QPointer<QValueAxis>>& GetListAxis();
private:
    QVector<DataSeriesACM> data_acm_;
    QVector<DataSeriesEtalon> data_etalon_;
    QVector<qreal> check_points_;
    QList<QPointer<QLabel>> vector_name_sensor_;
    QList<QPointer<QLabel>> vector_name_;
    QList<QPointer<QValueAxis>> list_axis_y_;
    QMap<QString,DataSeriesACM*> map_data_sensor_;
};

#endif // DATA_BASE_H
