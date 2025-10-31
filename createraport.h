#ifndef CREATERAPORT_H
#define CREATERAPORT_H
#pragma once
#include <data_base.h>

class CreateRaport
{
public:
    CreateRaport();
    void CreateAllDoc(QVector<DataSeriesACM>& vec_data);
    void CreateDoc(DataSeriesACM& data,QString name);
};

#endif // CREATERAPORT_H
