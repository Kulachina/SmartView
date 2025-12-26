#ifndef ERROR_TABLE_H
#define ERROR_TABLE_H
#pragma once
#include <QWidget>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableWidget>
#include "createraport.h"
#include "data_base.h"

class ErrorTable : public QWidget
{
    Q_OBJECT
public:
    ErrorTable(DataBase& data_base, CreateRaport& create_raport, QWidget *parent);
    void FillTable();
private:
    double CalcError(int type, double etalon, double data, int duration);
    void CreateAllDeltaDoc();
    void CreateDeltaDoc(QTableWidget* table, QString file_name,QString name_sensor);
    void FillEtalon(QTableWidget *table);
    void AnalisingSeries(DataSeriesSensor& data,QTableWidget *table);
    void DeleteTable();
    DataBase& data_base_;
    CreateRaport& create_raport_;
    QTabWidget *tab_;
    QVector<QTableWidget*> ptr_table;

};
#endif // ERROR_TABLE_H
