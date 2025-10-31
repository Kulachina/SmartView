#ifndef DATA_H
#define DATA_H
#pragma once
#include "qboxlayout.h"
#include <QVector>
#include <QDateTime>
#include <QLineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <QLabel>
#include <QStandardItemModel>
#include <QPointer>
#include <QCheckBox>

struct ACM{
    QLineSeries* series;
    QLabel* label;
    QLabel* label_data;
    QVector<QPointF> points_triangle;
    QVector<QPointF> points_rectangle;
    QValueAxis* axis;
    QHBoxLayout* hbox;
    QCheckBox* check_box;
    QPointer<QStandardItemModel> model;
    QString name_type;
    QString name_chart;
    QString name_unit;
    double unit_min = 0;
    double unit_max = 0;
    bool first_unit = true;
    bool first_write_rectangle = false;
};


struct DataSeriesACM{
    QString name_sensor;
    QVector<ACM> vec_acm;
    QFrame *line;
    QLabel *label_sensor;

    /*QLineSeries *series_temp;
    QLineSeries *series_bar;
    QLabel *data_sensor_temp;
    QLabel *data_sensor_bar;


    QPointer<QStandardItemModel> model_temp;
    QPointer<QStandardItemModel> model_bar;
    QHBoxLayout *hbox_temp;
    QHBoxLayout *hbox_bar;
    QCheckBox *check_temp;
    QCheckBox *check_bar;

    QVector<QPointF> points_triangle_view_bar;
    QVector<QPointF> points_rectangle_view_bar;
    QVector<QPointF> points_triangle_view_temp;
    QVector<QPointF> points_rectangle_view_temp;*/
};
struct DataEtalon{
    char magic[4];
    qint64 time;
    double value_1;
    double value_2;
    bool gap_series_1 = false;
    bool gap_series_2 = false;
    bool check_point = false;
};
struct DataHeaderEtalon{
    char magic[4];
    QString header_1;
    QString header_2;
};

struct DataSeriesEtalon{
    QString name_series;
    QLineSeries *series;
    QValueAxis *axis_y_;
    QScatterSeries *point_series;
    QList<QLineSeries*> old_series;
    QLabel *label_point,
           *data_sensor;
    QVector<QPointF> points_triangle_view_bar;
    QVector<QPointF> points_rectangle_view_bar;
    QVector<QPointF> points_triangle_view_temp;
    QVector<QPointF> points_rectangle_view_temp;
};
struct NameChart{
    QString name;
    int index;
    QString name_type;
    QString name_unit;
    QString name_sensor;
    double unit_min = 0;
    double unit_max = 0;
    bool first_unit = true;
};

#endif // DATA_H
