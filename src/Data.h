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
#include <QPair>

struct Canal{
    QLineSeries* series;
    QValueAxis *axis_y_;
    QLabel* label;
    QLabel* label_data;
    QLabel* label_delta;
    QLabel* label_name_canal;
    QLabel* label_name_sensor;
    QHBoxLayout* hbox;
    QCheckBox* check_box;
    QVector<QPointF> points_triangle;
    QVector<QPointF> points_rectangle;
    QVector<double> check_points;
    QVector<double> delta_points;
    QVector<QPair<double,double>> vec_max_min_unit;
    QPointer<QStandardItemModel> model;
    QString name_canal;
    QString new_name_canal;
    QString name_sensor;
    QString name_unit;
    QString color_series_RGB;
    QString color_series_;
    int type_error;
    int duration_error_min = 0;
    int duration_error_max = 0;
    double accept_min;
    double accept_max;
    double unit_min = 0;
    double unit_max = 0;
    bool first_unit = true;
    bool first_write_rectangle = false;
    bool select_box =false;
    bool flag_setting_canal = false;
    bool check_ACP = false;
};
struct DataSeriesSensor{
    QString name_sensor;
    QString number_sensor;
    QVector<Canal> vec_canal;
    QFrame *line;
    QLabel *label_sensor;
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
    QVector<QPointF> points_triangle_view;
    QVector<QPointF> points_rectangle_view;
};
struct NameChart{
    QString name_canal;//1
    QString name_unit;//2
    QString name_sensor;//4+5
};

#endif // DATA_H
