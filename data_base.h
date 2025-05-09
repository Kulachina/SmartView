#ifndef DATA_BASE_H
#define DATA_BASE_H
#include "Data.h"

#include <QMap>

class DataBase {
public:
    DataBase();
    QVector<Data>& GetData();
    void AddDataSerEtalon(DataSeriesEtalon data);
    QVector<DataSeriesEtalon>& GetDataSerEtalon();
private:
    QVector<Data> data_sensor_;
    QVector<DataSeriesEtalon> data_series_etalon_;
    QMap<QString,Data*> map_data_sensor_;
};

#endif // DATA_BASE_H
