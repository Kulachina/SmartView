#include "chartview.h"

ChartView::ChartView(QChartView *parent, DataBase& data_base)
    :QChartView(parent),
    data_base_(data_base),
    band_(QRubberBand::Rectangle,this)

{
    setMouseTracking(true);
    chart_ = new QChart();
    axis_time_ = new QDateTimeAxis();
    axis_time_->setTitleText("Время");
    axis_time_->setFormat("hh.mm.ss");
    axis_time_->setRange(QDateTime::currentDateTime(), QDateTime::currentDateTime().addSecs(600));
    axis_time_->setTickCount(21);
    axis_time_->setLabelsAngle(90);
    axis_temp_ = new QValueAxis();
    axis_temp_->setTitleText("Температура, °C");
    axis_bar_ = new QValueAxis();
    axis_bar_->setTitleText("Давление, кг/см2");
    axis_bar_->setVisible(false);
    axis_temp_->setVisible(false);
    axis_bar_->setRange(0, 0);
    axis_bar_->setTickCount(21);
    axis_temp_->setRange(0, 0);
    axis_temp_->setTickCount(21);
    chart_->addAxis(axis_time_,Qt::AlignBottom);
    chart_->addAxis(axis_temp_,Qt::AlignLeft);
    chart_->addAxis(axis_bar_,Qt::AlignLeft);
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
    line_from_mouse_->attachAxis(axis_time_);
    line_from_mouse_->attachAxis(axis_temp_);
    line_from_mouse_->setName("line_from_mouse");
    data_base_.AddListAxis(axis_temp_);
    data_base_.AddListAxis(axis_bar_);
}
QChart* ChartView::GetChart(){
    return chart_;
}
QDateTimeAxis* ChartView::GetAxisX(){
    return axis_time_;
}
QValueAxis* ChartView::GetAxisBar(){
    return axis_bar_;
}
QValueAxis* ChartView::GetAxisTemp(){
    return axis_temp_;
}
QWidget* ChartView::GetWidgetLegend(){
    return widget_legend_;
}
void ChartView::PanelLegendACM(){
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
    hbox_temp->addWidget(leg_temp);
    hbox_bar->addWidget(leg_bar);
    hbox_temp->addWidget(l_name_temp);
    hbox_temp->addWidget(data.data_sensor_temp);
    hbox_bar->addWidget(l_name_bar);
    hbox_bar->addWidget(data.data_sensor_bar);
    vbox_legend_->addLayout(hbox_temp);
    vbox_legend_->addLayout(hbox_bar);
    data_base_.AddLabelSensor(l_name_temp,leg_temp);
    data_base_.AddLabelSensor(l_name_bar,leg_bar);
    CreateMapLabel();
    CreateMapSeries();
}
void ChartView::PanelLegendEtalon(){
    QList<QLineSeries*> list_series;
    QHBoxLayout *hbox_time = new QHBoxLayout();
    QLabel *time = new QLabel();
    hbox_time->addWidget(new QLabel("- Время:"));
    hbox_time->addWidget(time);
    vbox_legend_->addLayout(hbox_time);
    for(auto data : data_base_.GetDataSerEtalon()){
        QHBoxLayout *hbox = new QHBoxLayout();
        QLabel *l_name = new QLabel(data.name_series);
        QLabel *leg = new QLabel();
        if(data.name_series == "ЛТ300" || data.name_series == "Имитатор ЛТ300" ){
            leg->setText("- Температура");
            leg->setStyleSheet("color: blue");
            l_name->setStyleSheet("color: blue");
            axis_temp_etalon_ = data.axis_y_;
        }
        if(data.name_series == "ДМ5002М" || data.name_series == "Имитатор ДМ5002М"){
            leg->setText("- Давление");
            leg->setStyleSheet("color: red");
            l_name->setStyleSheet("color: red");
            axis_bar_etalon_ = data.axis_y_;
        }
        hbox->addWidget(leg);
        hbox->addWidget(l_name);
        hbox->addWidget(data.data_sensor);
        vbox_legend_->addLayout(hbox);
        list_series << data.series;
        data_base_.AddLabelSensor(l_name,leg);
    }
    for(auto data : data_base_.GetDataSerEtalon()){
        QPointer<QLabel> point = data.data_sensor;
        map_data_label_[data.series->name()] = point;
    }
    QPointer<QLabel> point = time;
    map_data_label_["time"] = point;
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
    if(event->button() == Qt::LeftButton){
        last_pos_mouse_ = event->pos();
        band_.setGeometry(QRect(last_pos_mouse_, QSize()));
        band_.show();
    }
    if(event->button() == Qt::LeftButton && shift_series_){
        for(QAbstractSeries* abstract_series : chart_->series()){
            if(abstract_series->name() != "check_series" && abstract_series->name() != "Имитатор ЛТ300" && abstract_series->name() != "Имитатор ДМ5002М" && abstract_series->name() != "ЛТ300" && abstract_series->name() != "ДМ5002М"){
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
    }
    QChartView::mousePressEvent(event);
}
void ChartView::mouseMoveEvent(QMouseEvent *event){
    if(event->buttons() & Qt::RightButton){
        move_ = false;
        is_dragging_ = true;
    }
    if(band_.isVisible()){
        QRectF plot_area = chart_->plotArea();
        QPoint p1(last_pos_mouse_.x(), plot_area.top());
        QPoint p2(event->pos().x(), plot_area.bottom());
        QRect rect = QRect(p1, p2).normalized();
        band_.setGeometry(rect.normalized());
    }
    if(shift_series_){
        change_cursor_ = false;
        for(QAbstractSeries* abstract_series : chart_->series()){
            if(abstract_series->name() != "check_series" && abstract_series->name() != "Имитатор ЛТ300" && abstract_series->name() != "Имитатор ДМ5002М" && abstract_series->name() != "ЛТ300" && abstract_series->name() != "ДМ5002М"){
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
        QList<QAbstractAxis*> axes = chart()->axes(Qt::Vertical);
        if (!axes.isEmpty()) {
            QValueAxis* axisY = qobject_cast<QValueAxis*>(axes.first());
            if (axisY) {
                double y_min = axisY->min();
                double y_max = axisY->max();
                line_from_mouse_->clear();
                *line_from_mouse_ << QPointF(mouse_x, y_min) << QPointF(mouse_x, y_max);
            }
        }
        chart()->update();
        for(QAbstractSeries *series : chart_->series()){
            if(series != line_from_mouse_ && line_from_mouse_){
                QLineSeries *series_line = qobject_cast<QLineSeries*>(series);
                if(series_line){
                    for(QPointF point :series_line->points()){
                        QPoint screen_pos = chart_->mapToPosition(point,series).toPoint();
                        if(event->pos().x() == screen_pos.x()){
                            QDateTime time = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(point.x()));
                            QString time_str = time.toString("dd.MM.yyyy hh:mm:ss");
                            map_data_label_.value("time")->setText(time_str);
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
    if(event->button() == Qt::LeftButton && band_.isVisible()){
        band_.hide();
        if(last_pos_mouse_.x() > event->pos().x()){
            ResetZoom();
        } else {
            QRectF plot_area = chart_->plotArea();
            QPoint p1(last_pos_mouse_.x(), plot_area.top());
            QPoint p2(event->pos().x(), plot_area.bottom());
            QRect rect = QRect(p1, p2).normalized();
            ZoomChart(rect);
        }
    }
    chart_->update();
}
void ChartView::SaveZoom(){
    BoxZoom box;
    box.bar_max_acm_ = axis_bar_->max();
    box.bar_min_acm_ = axis_bar_->min();
    box.temp_max_acm_ = axis_temp_->max();
    box.temp_min_acm_ = axis_temp_->min();
    box.temp_min_etalon_ = axis_temp_etalon_->min();
    box.temp_max_etalon_ = axis_temp_etalon_->max();
    box.bar_min_etalon_ = axis_bar_etalon_->min();
    box.bar_max_etalon_ = axis_bar_etalon_->max();
    box.axis_max = axis_time_->max();
    box.axis_min = axis_time_->min();
    box_zoom_.push_back(box);
}
void ChartView::ResetZoom(){
    if(box_zoom_.empty()){
        return;
    }
    BoxZoom box = box_zoom_.back();
    axis_bar_->setRange(box.bar_min_acm_,box.bar_max_acm_);
    axis_temp_->setRange(box.temp_min_acm_, box.temp_max_acm_);
    axis_time_->setRange(box.axis_min,box.axis_max);
    axis_bar_etalon_->setRange(box.bar_min_etalon_,box.bar_max_etalon_);
    axis_temp_etalon_->setRange(box.temp_min_etalon_,box.temp_max_etalon_);
    box_zoom_.pop_back();

}
void ChartView::ZoomChart(QRect rect){
    SaveZoom();
    bool axis_min = false;
    bool axis_max = false;
    bool first_bar_etalon = false;
    bool first_temp_etalon = false;
    bool first_bar_acm = false;
    bool first_temp_acm = false;
    for(QAbstractSeries *series : chart_->series()){
        if(QLineSeries *series_line = qobject_cast<QLineSeries*>(series)){
            for(QPointF point :series_line->points()){
                QPoint screen_pos = chart_->mapToPosition(point,series).toPoint();
                if(rect.left() < screen_pos.x() && rect.right() > screen_pos.x()){
                    double y = point.y();
                    if(series->name() == "ЛТ300"){
                        if(!first_temp_etalon){
                            temp_max_etalon_ = y;
                            temp_min_etalon_ = y;
                            first_temp_etalon = true;
                        }
                        temp_max_etalon_ = qMax(temp_max_etalon_,y);
                        temp_min_etalon_ = qMin(temp_min_etalon_,y);
                    }
                    if(series->name() == "ДМ5002М"){
                        if(!first_bar_etalon){
                            bar_max_etalon_ = y;
                            bar_min_etalon_ = y;
                            first_bar_etalon = true;
                        }
                        bar_max_etalon_ = qMax(bar_max_etalon_,y);
                        bar_min_etalon_ = qMin(bar_min_etalon_,y);
                    }
                    if(series->name().endsWith("bar")){
                        if(!first_bar_acm){
                            bar_max_acm_ = y;
                            bar_min_acm_ = y;
                            first_bar_acm= true;
                        }
                        bar_max_acm_ = qMax(bar_max_acm_,y);
                        bar_min_acm_ = qMin(bar_min_acm_,y);
                    }
                    if(series->name().endsWith("temp")){
                        if(!first_temp_acm){
                            temp_max_acm_ = y;
                            temp_min_acm_ = y;
                            first_temp_acm= true;
                        }
                        temp_max_acm_ = qMax(temp_max_acm_,y);
                        temp_min_acm_ = qMin(temp_min_acm_,y);
                    }
                }
                if(rect.left() == screen_pos.x() && !axis_min){
                    QDateTime time = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(point.x()));
                    axis_min_ = time;
                    axis_min = true;
                }
                if(rect.right() == screen_pos.x() && !axis_max){
                    QDateTime time = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(point.x()));
                    axis_max_ = time;
                    axis_max = true;
                }
            }
        }
    }
    axis_bar_etalon_->setRange(bar_min_etalon_,bar_max_etalon_ + bar_max_etalon_*0.1);
    axis_temp_etalon_->setRange(temp_min_etalon_,temp_max_etalon_ + temp_max_etalon_*0.1);
    axis_time_->setRange(axis_min_,axis_max_);
    axis_bar_->setRange(bar_min_acm_,bar_max_acm_ + bar_max_acm_*0.05);
    axis_temp_->setRange(temp_min_acm_,temp_max_acm_ + temp_max_acm_*0.05);
}
