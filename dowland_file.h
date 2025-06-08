#ifndef DOWLAND_FILE_H
#define DOWLAND_FILE_H

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
    void LoadDocACM(QString path);
    void CreateSeriesACM(QStringList words);
    void CreateSeriesEtalon(QStringList words);
    void AddDataEtalon(DataEtalon data);
    void AddDataACM(QStringList words);
    void SetAxisTime(QDateTimeAxis* axis_x);
    void SetChartDoc(QChart* chart,QValueAxis* axis_temp,QValueAxis* axis_bar);
    void GapSeries(DataSeriesEtalon& doc);
    void CreateHeader();
    void ClearTable();
    void CheckFlag();
    void LoadDocEtalon(QString path);
    QVector<DataSeriesEtalon>& GetDataSeriesEtalon();
    QDateTime GetAxisTime();
    void ClearAll();
private:
    void SetMinMaxY(double temp, double bar);
    qint64 TextToInt(QString time);
    DataBase& data_base_;
    QVector<DataSeriesEtalon> data_etalon_;
    QVector<DataSeriesACM> data_acm_;
    QVector<QPointF> p_bar_;
    QVector<QPointF> p_temp_;
    QPointer<QDateTimeAxis> axis_x_;
    QDateTime now_time_;
    QPointer<QChart> chart_;
    QPointer<QValueAxis> axis_temp_,
                         axis_bar_;
    QFile file_;
    QProgressBar *progress_;
    QWidget *w_progress_;
    int count = 0;
    double bar_max_ = 0,
        bar_min_ = 0,
        temp_max_ = 0,
        temp_min_ = 0;
    bool set_axis_x_ = false,
         error_flag_1_ = true,
         error_flag_2_ = true,
         create_file_ = false,
         create_title_ = false,
         gap_ = false,
         first_min_max_ = false;
};

#endif // DOWLAND_FILE_H
