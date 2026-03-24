#ifndef DOWLAND_FILE_H
#define DOWLAND_FILE_H
#pragma once
#include "data_base.h"
#include <QProgressBar>
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
#include <QPointer>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDataStream>
#include <QStandardPaths>


class DowlandFile
{
public:
    DowlandFile(DataBase& data_base);
    void CreateACM(DataSeriesSensor& data, QString name, QString name_canal, QString name_unit);
    void LoadSVDoc(const QString path);
    void SaveSVDoc(const QString path);
    DataSeriesSensor LoadDocACM(QString path, int count_file, int count_now);
    DataSeriesSensor LoadDocAMT(QString path, QString name, int count_file, int count_now);
    void CreateSeriesACM(DataSeriesSensor& data);
    void CreateSeriesEtalon(QString word);
    void AddDataEtalon(DataEtalon data);
    void AddDataACM(QStringList words,DataSeriesSensor& data);
    void SetAxisTime(QDateTimeAxis* axis_x);
    void SetChartDoc(QChart* chart,QValueAxis* axis_temp,QValueAxis* axis_bar);
    void GapSeries(DataSeriesEtalon& doc);
    void CreateHeader();
    void ClearTable();
    void CheckFlag();
    void LoadDocEtalon(QString path);
    void LoadDocEtalon_2v(QString path);
    QVector<DataSeriesEtalon>& GetDataSeriesEtalon();
    QDateTime GetAxisTime();
    void ClearAll();
    bool SelectChart(QStringList& words);
private:
    void CreateVecCheckPoints(QString name, QList<QPointF> list);
    void LoadDataEt(QDataStream& in);
    void LoadDataACM(QDataStream& in);
    void SaveDataEt(QDataStream& out);
    void SaveDataACM(QDataStream& out);
    void SetMinMaxY(double temp, double bar);
    qint64 TextToInt(QString time);
    DataBase& data_base_;
    QVector<DataSeriesEtalon> data_etalon_;
    QVector<DataSeriesSensor> data_acm_;
    QVector<QPointF> p_bar_;
    QVector<QPointF> p_temp_;
    QVector<QPointF> check_points_bar_;
    QVector<QPointF> check_points_temp_;
    QPointer<QDateTimeAxis> axis_x_;
    QDateTime now_time_;
    QPointer<QChart> chart_;
    QPointer<QValueAxis> axis_temp_,
                         axis_bar_;
    QList<QCheckBox*> check_list_;
    QVector<NameChart> chart_name_and_index_;
    QFile file_;
    QProgressBar *prog_;
    QWidget *w_select_chart_;
    int count = 0,
        index_data_;
    double bar_max_ = 0,
        bar_min_ = 0,
        temp_max_ = 0,
        temp_min_ = 0;
    bool set_axis_x_ = false,
         error_flag_1_ = true,
         error_flag_2_ = true,
         create_file_ = false,
         create_title_ = false,
        first_write_rectangle_ =false,
         gap_ = false,
         first_min_max_ = false;
};

#endif // DOWLAND_FILE_H
