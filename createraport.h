#ifndef CREATERAPORT_H
#define CREATERAPORT_H
#pragma once
#include <data_base.h>

class CreateRaport
{
public:
    CreateRaport(DataBase& data_base);
    void CreateAllDoc(QVector<DataSeriesACM>& vec_data);
    void CreateDoc(DataSeriesACM& data,QString name);
    void CreateAllDeltaDoc();
    void CreateDeltaDoc(DataSeriesACM& acm,QString file_name,QString name_sensor);
private:
    DataBase& data_base_;
};

#endif // CREATERAPORT_H
