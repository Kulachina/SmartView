#ifndef ERROR_TABLE_H
#define ERROR_TABLE_H
#pragma once
#include <QWidget>
#include <QStandardItemModel>
#include <QStandardItem>
#include "createraport.h"
#include "data_base.h"

class ErrorTable : public QWidget
{
    Q_OBJECT
public:
    ErrorTable(DataBase& data_base, CreateRaport& create_raport, QWidget *parent);
    void FillTable();
    void CreateRapor();

private:
    void AnalisingSeries(DataSeriesACM data);
    QStandardItemModel *model_;
    DataBase& data_base_;
    CreateRaport& create_raport_;



};
#endif // ERROR_TABLE_H
