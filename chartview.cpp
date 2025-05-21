#include "chartview.h"

ChartView::ChartView(QChartView *parent, DataBase& data_base)
    :QChartView(parent),
    data_base_(data_base)

{
    chart_ = new QChart();
    axis_time_ = new QDateTimeAxis();
    axis_time_->setTitleText("Время");
    axis_time_->setFormat("hh.mm.ss");
    axis_time_->setRange(QDateTime::currentDateTime(), QDateTime::currentDateTime().addSecs(600));
    axis_time_->setTickCount(21);
    axis_time_->setLabelsAngle(90);
    QValueAxis *axis_y = new QValueAxis();
    axis_y->setTitleVisible(false);
    axis_y->setRange(0, 500);
    axis_y->setTickCount(21);
    axis_y->setLabelsAngle(90);
    axis_y->setTitleVisible(false);
    axis_y->setLabelsVisible(false);
    chart_->addAxis(axis_time_,Qt::AlignBottom);
    chart_->addAxis(axis_y,Qt::AlignLeft);
    chart_->legend()->setVisible(false);
    chart_->setActive(true);
    setChart(chart_);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing);
    widget_legend_ = new QWidget();
    vbox_legend_ = new QVBoxLayout();
    vbox_legend_->setAlignment(Qt::AlignTop);
    widget_legend_->setLayout(vbox_legend_);
    line_from_mouse_ = new QLineSeries();
    chart_->addSeries(line_from_mouse_);
}
QChart* ChartView::GetChart(){
    return chart_;
}
QDateTimeAxis* ChartView::GetAxisX(){
    return axis_time_;
}
QWidget* ChartView::GetWidgetLegend(){
    return widget_legend_;
}
void ChartView::PanelLegend(){
    DataSeriesACM data = data_base_.GetDataSerACM().back();
    QHBoxLayout *hbox_temp = new QHBoxLayout();
    QHBoxLayout *hbox_bar = new QHBoxLayout();
    QLabel *leg_bar = new QLabel("- Давление");
    QLabel *leg_temp = new QLabel("- Температура");
    QLabel *l_name_temp = new QLabel(data.name_sensor);
    l_name_temp->setStyleSheet("color: blue");
    QLabel *l_name_bar = new QLabel(data.name_sensor);
    l_name_bar->setStyleSheet("color: red");
    leg_bar->setStyleSheet("color: red");
    leg_temp->setStyleSheet("color: blue");
    hbox_bar->addWidget(leg_bar);
    hbox_temp->addWidget(leg_temp);
    hbox_temp->addWidget(l_name_temp);
    hbox_temp->addWidget(data.data_sensor_temp);
    hbox_bar->addWidget(l_name_bar);
    hbox_bar->addWidget(data.data_sensor_bar);
    vbox_legend_->addLayout(hbox_bar);
    vbox_legend_->addLayout(hbox_temp);
    CreateMapSeries();
    CreateMapLabel();
}
void ChartView::CreateLegend(QString name, QList<QLineSeries*> series){
    for(QLineSeries* s : series){
        QHBoxLayout *hbox = new QHBoxLayout();
        QCheckBox *check = new QCheckBox();
        QLabel *label = new QLabel();
        if(s->name() == "bar"){
            label->setText("Давление " + name);
        }
        if(s->name() == "temp"){
            label->setText("Темпертура " + name);
        }
        hbox->addWidget(check);
        hbox->addWidget(label);
        vbox_legend_->addLayout(hbox);
    }
}
void ChartView::CreateMapSeries(){
    DataSeriesACM data =data_base_.GetDataSerACM().back();
    QList<QLineSeries*> list_series;
    list_series << data.series_bar << data.series_temp;
    map_series_[data.series_temp->name()] = list_series;
    map_series_[data.series_bar->name()] = list_series;
}
void ChartView::CreateMapLabel(){
    DataSeriesACM data =data_base_.GetDataSerACM().back();
    QPointer<QLabel> point_temp = data.data_sensor_temp;
    QPointer<QLabel> point_bar = data.data_sensor_bar;
    map_data_label_[data.series_bar->name()] = point_bar;
    map_data_label_[data.series_temp->name()] = point_temp;
}
void ChartView::ToogledFlagShiftSeries(){
    if(shift_series_){
        shift_series_ = false;
    } else {
        shift_series_ = true;
    }
}
void ChartView::ToogledFlagLineInMouse(){
    if(data_in_time_){
        data_in_time_ = false;
        line_from_mouse_->hide();
    } else {
        data_in_time_ = true;
    }
}
void ChartView::MoveSeries(QLineSeries* series, qreal dx){
    QList<QPointF> points = series->points();
    for(QPointF& p : points){
        p.setX(p.x() + dx);
    }
    series->replace(points);
}
void ChartView::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::RightButton){
        move_ = true;
        is_dragging_ = false;
        last_pos_mouse_ = event->pos();
    }
    if(event->button() == Qt::LeftButton && shift_series_){
        for(QAbstractSeries* abstract_series : chart_->series()){
            QLineSeries *series = qobject_cast<QLineSeries*>(abstract_series);
            for(QPointF& p : series->points()){
                QPoint screen_pos = chart_->mapToPosition(p,series).toPoint();
                if(QRect(screen_pos - QPoint(5,5),QSize(10,10)).contains(event->pos())){
                    active_series_ = map_series_[series->name()];
                    is_dragging_series_ = true;
                    last_pos_mouse_ = event->pos();
                    return;
                }
            }
        }
    }
    QChartView::mousePressEvent(event);
}
void ChartView::mouseMoveEvent(QMouseEvent *event){
    if(event->buttons() & Qt::RightButton){
        move_ = false;
        is_dragging_ = true;
    }
    if(shift_series_){
        change_cursor_ = false;
        for(QAbstractSeries* abstract_series : chart_->series()){
            QLineSeries *series = qobject_cast<QLineSeries*>(abstract_series);
            for(QPointF& p : series->points()){
                QPoint screen_pos = chart_->mapToPosition(p,series).toPoint();
                if(QRect(screen_pos - QPoint(5,5),QSize(10,10)).contains(event->pos())){
                    setCursor(Qt::PointingHandCursor);
                    change_cursor_ = true;
                    break;
                }
            }
            if(change_cursor_){
                break;
            }
        }
        if(!change_cursor_){
           unsetCursor();
        }
    }
    if(is_dragging_series_){
        QPointF last = chart_->mapToValue(last_pos_mouse_,active_series_[0]);
        QPointF now = chart_->mapToValue(event->pos(),active_series_[0]);
        qreal dx = now.x() - last.x();
        for(QLineSeries* series : active_series_){
            MoveSeries(series, dx);
        }
        last_pos_mouse_ = event->pos();
    }
    if(is_dragging_){
        QPointF  delta = mapToScene(event->pos()) - mapToScene(last_pos_mouse_);
        chart_->scroll(-delta.x(),0);
        last_pos_mouse_ = event->pos();
    }
    if(data_in_time_){
        line_from_mouse_->show();
        QPointF mouse_pos = chart_->mapToValue(event->pos());
        double mouse_x = mouse_pos.x();
        QList<QAbstractAxis*> axes = chart_->axes(Qt::Vertical);
        if (!axes.isEmpty()) {
            QValueAxis* axisY = qobject_cast<QValueAxis*>(axes.back());
            if (axisY) {
                double y_min = axisY->min();
                double y_max = axisY->max();
                line_from_mouse_->clear();
                *line_from_mouse_ << QPointF(mouse_x, y_min) << QPointF(mouse_x, y_max);
            }
        }
        chart_->update();
        for(QAbstractSeries *series : chart_->series()){
            if(series != line_from_mouse_ && line_from_mouse_){
                QLineSeries *series_line = qobject_cast<QLineSeries*>(series);
                if(series_line){
                    for(QPointF point :series_line->points()){
                        QPoint screen_pos = chart_->mapToPosition(point,series).toPoint();
                        if(event->pos().x() == screen_pos.x()){
                            map_data_label_.value(series->name())->setText(QString::number(point.y()));
                        }
                    }
                }
            }
        }
    }
    QChartView::mouseMoveEvent(event);
}
void ChartView::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::RightButton){
        is_dragging_ = false;
    }
    if(event->button() == Qt::LeftButton){
        is_dragging_series_ = false;
    }
    chart_->update();
}
