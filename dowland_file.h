#ifndef DOWLAND_FILE_H
#define DOWLAND_FILE_H

#include "data_base.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QLineSeries>
#include <QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QChart>
#include <QDateTime>
#include <QLabel>
#include <QStandardItemModel>
#include <QScatterSeries>

class DowlandFile
{
public:
    DowlandFile(DataBase& data_base);
    void LoadDocACM(QString path);
    void CreateSeriesACM(QStringList words);
    void CreateSeriesEtalon(QStringList words);
    void AddDataEtalon(DataEtalon data);
    void AddDataACM(QStringList words);
    void SetAxisTime(QDateTimeAxis* axis_x);
    void SetChartDoc(QChart* chart);
    void GapSeries(DataSeriesEtalon& doc);
    void CreateHeader();
    void ClearTable();
    void CheckFlag();
    void LoadDocEtalon(QString path);
    void SetAxisY(QValueAxis* data, double y);
    QVector<DataSeriesEtalon>& GetDataSeriesEtalon();
    QDateTime GetAxisTime(); 
private:
    qint64 TextToInt(QString time);
    DataBase& data_base_;
    QVector<DataSeriesEtalon> data_etalon_;
    QVector<DataSeriesACM> data_acm_;
    QVector<QPointF> p_bar_;
    QVector<QPointF> p_temp_;
    QDateTimeAxis *axis_x_;
    QDateTime now_time_;
    QChart* chart_;
    QFile file_;
    bool set_axis_x_ = false,
         error_flag_1_ = true,
         error_flag_2_ = true,
         create_file_ = false,
         create_title_ = false,
         gap_ = false,
         first_min_max_ = false;
};

#endif // DOWLAND_FILE_H
