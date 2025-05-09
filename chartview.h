#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QChartView>
#include <QObject>
#include <QDateTimeAxis>
#include <QDateTime>
#include <QValueAxis>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineSeries>
#include <QCheckBox>

#include "data_base.h"

class ChartView : public QChartView
{
    Q_OBJECT
public:
    ChartView(QChartView *parent, DataBase& data_base);
    QWidget* GetWidgetLegend();
    void CreateSeries();
    void CreateLegend(QString name, QList<QLineSeries*> series);
    QChart* GetChart();
    QDateTimeAxis* GetAxisX();
protected slots:
    //void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override ;
    void mouseMoveEvent(QMouseEvent *event) override ;
    void mouseReleaseEvent(QMouseEvent *event) override ;
    //void leaveEvent(QEvent *event) override;
    //void enterEvent(QEnterEvent *event) override;
private:
    QChartView *chart_view_;
    QChart *chart_;
    QWidget *widget_legend_;
    DataBase& data_base_;
    QDateTimeAxis *axis_time_;
    QVBoxLayout *vbox_legend_;
    QPoint last_pos_mouse_;
    bool move_ = false,
         is_dragging_ = false;
};

#endif // CHARTVIEW_H
