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
    widget_legend_->setLayout(vbox_legend_);
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
void ChartView::CreateSeries(){
    Data data = data_base_.GetData().back();
    QLineSeries *s_bar = new QLineSeries();
    QLineSeries *s_temp = new QLineSeries();
    QValueAxis *axis_bar = new QValueAxis();
    QValueAxis *axis_temp = new QValueAxis();
    s_bar->setName("bar");
    s_bar->attachAxis(axis_bar);
    s_bar->attachAxis(axis_time_);
    s_temp->setName("temp");
    s_temp->attachAxis(axis_temp);
    s_temp->attachAxis(axis_time_);
    chart_->addSeries(s_bar);
    chart_->addSeries(s_temp);
    for(int i = 0; i < data.temp.size(); ++i){
        s_temp->append(data.time[i],data.temp[i]);
        s_bar->append(data.time[i],data.bar[i]);
    }
    QList<QLineSeries*> group_sensor_series = {s_bar, s_temp};
    CreateLegend(data.name_sensor, group_sensor_series);
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
void ChartView::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        move_ = true;
        is_dragging_ = false;
        last_pos_mouse_ = event->pos();
    }
    QChartView::mousePressEvent(event);
}
void ChartView::mouseMoveEvent(QMouseEvent *event){
    if(event->buttons() & Qt::LeftButton){
        move_ = false;
        is_dragging_ = true;
    }
    if(is_dragging_){
        QPointF  delta = mapToScene(event->pos()) - mapToScene(last_pos_mouse_);
        chart_->scroll(-delta.x(),0);
        last_pos_mouse_ = event->pos();
    }
    QChartView::mouseMoveEvent(event);
}
void ChartView::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        is_dragging_ = false;
    }
    chart_->update();
}
