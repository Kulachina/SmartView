#ifndef ERROR_TABLE_H
#define ERROR_TABLE_H
#pragma once
#include <QWidget>
#include <QStandardItemModel>
#include <QStandardItem>

#include "data_base.h"

class ErrorTable : public QWidget
{
    Q_OBJECT
public:
    ErrorTable(DataBase& data_base, QWidget *parent);
    void FillTable();

private:
    void AnalisingSeries(DataSeriesACM data);
    QStandardItemModel *model_;
    DataBase& data_base_;



};
#endif // ERROR_TABLE_H
