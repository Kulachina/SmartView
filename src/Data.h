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
#include <QDataStream>

struct Canal{
    QLineSeries* series = nullptr;
    QValueAxis *axis_y_ = nullptr;
    QLabel* label = nullptr;
    QLabel* label_data = nullptr;
    QLabel* label_delta = nullptr;
    QLabel* label_name_canal = nullptr;
    QLabel* label_name_sensor = nullptr;
    QHBoxLayout* hbox = nullptr;
    QCheckBox* check_active_canal = nullptr;
    QVector<QPointF> points_triangle;
    QVector<QPointF> points_rectangle;
    QVector<double> check_points;
    QVector<double> delta_points;
    QVector<QPair<double,double>> vec_max_min_unit;
    QPointer<QCheckBox> check_box;
    QStandardItemModel* model = nullptr;
    QString first_name_canal;
    QString name_canal;
    QString new_name_canal;
    QString name_sensor;
    QString name_unit;
    QString color_series_RGB;
    QString color_series_;
    int type_error = 0;          // 0 — не задан, 1 абс., 2 отн., 3 прив.
    int duration_error_min = 0;
    int duration_error_max = 0;
    double accept_min = 0;
    double accept_max = 0;
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
    QFrame *line = nullptr;
    QLabel *label_sensor = nullptr;
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
    QLineSeries *series = nullptr;
    QValueAxis *axis_y_ = nullptr;
    QScatterSeries *point_series = nullptr;
    QList<QLineSeries*> old_series;
    QLabel *label_point = nullptr,
           *data_sensor = nullptr;
    QVector<QPointF> points_triangle_view;
    QVector<QPointF> points_rectangle_view;
    QVector<double> condition;
};
struct NameChart{
    QString name_canal;//1
    QString name_unit;//2
    QString name_sensor;//4+5
};
// Контрольный диапазон: отрезок [t_start, t_end] и средние значения эталонных
// кривых на нём (аналог контрольной точки, но усреднённый по отрезку).
struct CheckRange{
    QDateTime t_start;
    QDateTime t_end;
    QDateTime t_mid;
    double avg_temp = 0;
    double avg_bar = 0;
};

// Сериализация контрольного диапазона в документ .smv.
inline QDataStream& operator<<(QDataStream& out, const CheckRange& range){
    out << range.t_start << range.t_end << range.t_mid
        << range.avg_temp << range.avg_bar;
    return out;
}
inline QDataStream& operator>>(QDataStream& in, CheckRange& range){
    in >> range.t_start >> range.t_end >> range.t_mid
        >> range.avg_temp >> range.avg_bar;
    return in;
}

#endif // DATA_H
