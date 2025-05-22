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
#include <QPointer>
#include <QRubberBand>

#include "data_base.h"

class ChartView : public QChartView
{
    Q_OBJECT
public:
    ChartView(QChartView *parent, DataBase& data_base);
    QWidget* GetWidgetLegend();
    void CreateSeries();
    void CreateLegend(QString name, QList<QLineSeries*> series);
    void PanelLegendACM();
    void PanelLegendEtalon();
    QChart* GetChart();
    QDateTimeAxis* GetAxisX();
    void ToogledFlagShiftSeries();
    void ToogledFlagLineInMouse();
    QValueAxis* GetAxisBar();
    QValueAxis* GetAxisTemp();

protected slots:
    //void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override ;
    void mouseMoveEvent(QMouseEvent *event) override ;
    void mouseReleaseEvent(QMouseEvent *event) override ;
    //void leaveEvent(QEvent *event) override;
    //void enterEvent(QEnterEvent *event) override;
private:
    void MoveSeries(QLineSeries* series, qreal dx);
    void CreateMapSeries();
    void CreateMapLabel();
    QChartView *chart_view_;
    QChart *chart_;
    QValueAxis *axis_bar_,
               *axis_temp_;
    QWidget *widget_legend_;
    DataBase& data_base_;
    QDateTimeAxis *axis_time_;
    QVBoxLayout *vbox_legend_;
    QLineSeries* line_from_mouse_;
    QMap<QString,QList<QLineSeries*>> map_series_;
    QMap<QString,QLabel*> map_data_label_;
    QList<QLineSeries*> active_series_;
    QPoint last_pos_mouse_;
    QRect hit_area_;
    QRubberBand band_;
    bool move_ = false,
        is_dragging_ = false,
        is_dragging_series_ = false,
        shift_series_ = false,
        change_cursor_ = false,
        data_in_time_ = false;
};

#endif // CHARTVIEW_H
