#ifndef LAS_H
#define LAS_H
#pragma once
#include <QString>
#include <QVector>
#include <QCheckBox>
#include <QLineEdit>
#include <QProgressBar>
#include <QApplication>
#include "Data.h"


class Las
{
public:
    Las();
    DataSeriesSensor& DowlandLas(const QString path);
private:
    void SetNameSensor(const QStringList& words);
    void SetStartDate(const QStringList& words);
    void SetStartTime(const QStringList& words);
    void SelectCurve(QStringList& words);
    void LogData(const QStringList& data);
    void CreateSensor();
    void CreateCanal(QString name);
    bool select_curve_ = false,
         log_data_ = false;
    QString date_;
    QVector<QString> name_curve_;
    DataSeriesSensor data_;
    QProgressBar *prog_;
    int index_time_;
};

#endif // LAS_H
