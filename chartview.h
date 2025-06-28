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
#include <QtMath>
#include <QFrame>
#include "data_base.h"
struct BoxZoom{
    double bar_max_etalon_,
        bar_min_etalon_,
        bar_max_acm_,
        bar_min_acm_,
        temp_max_etalon_,
        temp_min_etalon_,
        temp_max_acm_,
        temp_min_acm_;
    QDateTime axis_min,
              axis_max;
};

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
    void ToogledFlagShiftCheckPoint();
    void ToogledFlagLineInMouse();
    QValueAxis* GetAxisBar();
    QValueAxis* GetAxisTemp();
    void ZoomOn();
    void ClearPanelLegend();
    void ReplaceCheckSeries();
    void ReBuildPointSeries();
protected slots:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    void AddSeparator();
    void SetLineFromMouse(QPointF point);
    void ZoomChart(QRect rect);
    void SaveZoom();
    void ResetZoom();
    void MoveSeries(QLineSeries* series, qreal dx);
    void MoveCheckPoint(qreal point, qreal dx);
    void CreateMapSeries();
    void CreateMapLabel();
    QChartView *chart_view_;
    QChart *chart_;
    QValueAxis *axis_bar_,
               *axis_temp_;
    QPointer<QValueAxis> axis_bar_etalon_,
                         axis_temp_etalon_;
    QWidget *widget_legend_;
    DataBase& data_base_;
    QDateTimeAxis *axis_time_;
    QVBoxLayout *vbox_legend_;
    QLineSeries* line_from_mouse_;
    QMap<QString,QList<QPointer<QLineSeries>>> map_series_;
    QMap<QString,QPointer<QLabel>> map_data_label_;
    QList<QPointer<QLineSeries>> active_series_;
    QPointer<QScatterSeries> active_check_series_;
    QPoint last_pos_mouse_;
    QRect hit_area_;
    QRubberBand band_;
    QDateTime axis_min_,
              axis_max_;
    QVector<BoxZoom> box_zoom_;
    bool move_ = false,
        is_dragging_ = false,
        is_dragging_series_ = false,
        is_dragging_check_point_ = false,
        shift_series_ = false,
        shift_check_point_ = false,
        change_cursor_ = false,
        data_in_time_ = false,
        load_etalon_ = false;
    double bar_max_etalon_,
        bar_min_etalon_,
        bar_max_acm_,
        bar_min_acm_,
        temp_max_etalon_,
        temp_min_etalon_,
        temp_max_acm_,
        temp_min_acm_,
        line_from_mouse_min_,
        line_from_mouse_max_ ;
    int active_check_point_;
};

#endif // CHARTVIEW_H
