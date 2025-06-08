#ifndef DATA_H
#define DATA_H
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

struct DataSeriesACM{
    QString name_sensor;
    QLineSeries *series_temp;
    QLineSeries *series_bar;
    QLabel *label_sensor;
    QLabel *data_sensor_temp;
    QLabel *data_sensor_bar;
    QPointer<QStandardItemModel> model_temp;
    QPointer<QStandardItemModel> model_bar;
    QHBoxLayout *hbox_temp;
    QHBoxLayout *hbox_bar;
    QCheckBox *check_temp;
    QCheckBox *check_bar;
    QFrame *line;
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
};

#endif // DATA_H
