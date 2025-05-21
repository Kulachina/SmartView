#ifndef DATA_BASE_H
#define DATA_BASE_H
#include "Data.h"

#include <QMap>

class DataBase {
public:
    DataBase();
    void AddDataSerEtalon(DataSeriesEtalon data);
    void AddDataSerACM(DataSeriesACM& data);
    QVector<DataSeriesACM>& GetDataSerACM();
    QVector<DataSeriesEtalon>& GetDataSerEtalon();
private:
    QVector<DataSeriesACM> data_acm_;
    QVector<DataSeriesEtalon> data_etalon_;
    QMap<QString,DataSeriesACM*> map_data_sensor_;
};

#endif // DATA_BASE_H
