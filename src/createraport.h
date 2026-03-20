#ifndef CREATERAPORT_H
#define CREATERAPORT_H
#pragma once
#include <data_base.h>
#include <QTableWidget>

class CreateRaport
{
public:
    CreateRaport(DataBase& data_base);
    void CreateAllDoc(QVector<DataSeriesSensor>& vec_data);
    void CreateAllShortDoc(QVector<DataSeriesSensor>& vec_data,QString name_canal);
    void CreateDoc(DataSeriesSensor& data,QString name);
    void CreateAllDeltaDoc(QTabWidget* tab);
    void CreateDeltaDoc(QTableWidget* table, QString file_name, QString name_sensor);
    void CreateShortDoc(DataSeriesSensor& data, QString name, QString name_canal);
private:
    DataBase& data_base_;
};

#endif // CREATERAPORT_H
