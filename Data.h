#ifndef DATA_H
#define DATA_H
#include <QVector>
#include <QDateTime>
#include <QLineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <QLabel>
struct DataSeriesACM{
    QString name_sensor;
    QLineSeries *series_temp;
    QLineSeries *series_bar;
    QValueAxis *axis_y_bar;
    QValueAxis *axis_y_temp;
    QLabel *label_sensor;
    QLabel *data_sensor_temp;
    QLabel *data_sensor_bar;
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
    QScatterSeries *point_series;
    QList<QLineSeries*> old_series;
    QLabel *label_point,
           *label_sensor,
           *data_sensor;
};

#endif // DATA_H
