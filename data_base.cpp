#include "data_base.h"

DataBase::DataBase()
{
    data_series_etalon_.reserve(50);
}

QVector<Data>& DataBase::GetData(){
    return data_sensor_;
}
void DataBase::AddDataSerEtalon(DataSeriesEtalon data){

    data_series_etalon_.push_back(data);
}
QVector<DataSeriesEtalon>& DataBase::GetDataSerEtalon(){
    return data_series_etalon_;
}
