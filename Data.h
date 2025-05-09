#ifndef DATA_H
#define DATA_H
#include <QVector>
#include <QDateTime>
#include <QLineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <QLabel>
struct Data{
    QString name_sensor;
    QVector<double> temp;
    QVector<double> bar;
    QVector<qint64> time;
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
    QValueAxis *axis_y;
    QLabel *label_point;
};

#endif // DATA_H
