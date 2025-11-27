#ifndef CREATERAPORT_H
#define CREATERAPORT_H
#pragma once
#include <data_base.h>

class CreateRaport
{
public:
    CreateRaport(DataBase& data_base);
    void CreateAllDoc(QVector<DataSeriesSensor>& vec_data);
    void CreateDoc(DataSeriesSensor& data,QString name);
    void CreateAllDeltaDoc();
    void CreateDeltaDoc(DataSeriesSensor& acm,QString file_name,QString name_sensor);
private:
    DataBase& data_base_;
};

#endif // CREATERAPORT_H
