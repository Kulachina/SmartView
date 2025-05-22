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
    QVector<DataSeriesACM>& GetDataSerACM();
    QVector<DataSeriesEtalon>& GetDataSerEtalon();
private:
    QVector<DataSeriesACM> data_acm_;
    QVector<DataSeriesEtalon> data_etalon_;
    QList<QPointer<QLabel>> vector_name_sensor_;
    QList<QPointer<QLabel>> vector_name_;
    QMap<QString,DataSeriesACM*> map_data_sensor_;
};

#endif // DATA_BASE_H
