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
    //data_base_.AddListAxis(axis_temp_);
    //data_base_.AddListAxis(axis_bar_);
    chart_->addAxis(axis_time_,Qt::AlignBottom);
    chart_->addAxis(axis_temp_,Qt::AlignLeft);
    chart_->addAxis(axis_bar_,Qt::AlignLeft);
    chart_->legend()->setVisible(false);
    chart_->setActive(true);
    setChart(chart_);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);
    widget_legend_ = new QWidget();
    vbox_legend_ = new QVBoxLayout();
    vbox_legend_->setAlignment(Qt::AlignTop);
    widget_legend_->setLayout(vbox_legend_);
    line_from_mouse_ = new QLineSeries();
    chart_->addSeries(line_from_mouse_);
    line_from_mouse_->attachAxis(axis_time_);
    line_from_mouse_->setName("line_from_mouse");
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
void ChartView::PanelLegendACM(DataSeriesSensor& data){
    QVector<Canal>& acm = data.vec_canal;
    data.line = new QFrame();
    data.line->setFrameShape(QFrame::HLine);
    data.line->setStyleSheet("background-color: grey");
    data.line->setFixedHeight(1);
    data.line->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    vbox_legend_->addWidget(data.line);
    for(int i =0;i < acm.size();++i){
        if(!acm[i].select_box){
            continue;
        }
        chart_->addAxis(acm[i].axis_y_,Qt::AlignLeft);
        chart_->addSeries(acm[i].series);
        acm[i].series->attachAxis(axis_time_);
        acm[i].series->attachAxis(acm[i].axis_y_);
        acm[i].series->replace(acm[i].points_triangle);
        acm[i].axis_y_->setRange(acm[i].unit_min,acm[i].unit_max);
        acm[i].axis_y_->setVisible(false);
        data_base_.AddListAxis(acm[i].axis_y_);
        acm[i].hbox = new QHBoxLayout();
        acm[i].hbox->setAlignment(Qt::AlignLeft);
        acm[i].check_box = new QCheckBox();
        acm[i].check_box->setCheckState(Qt::Checked);
        connect(acm[i].check_box, &QCheckBox::toggled, this,[=](){
            if(acm[i].check_box->isChecked()){
                acm[i].series->setVisible(true);
            } else {
                acm[i].series->setVisible(false);
            }
        });
        QPointer<QHBoxLayout> p_hbox = acm[i].hbox;
        QLabel *l_name_type = new QLabel(acm[i].name_type);
        QLabel *l_name = new QLabel(data.name_sensor);
        l_name_type->setStyleSheet("color: rgb(" + acm[i].color_series + ");");
        l_name->setStyleSheet("color: rgb(" + acm[i].color_series + ");");
        if(acm[i].color_series.isEmpty()){
            acm[i].color_series = "0,0,0";
        }
        QStringList p = acm[i].color_series.split(',');
        QColor col(
            p[0].toInt(),
            p[1].toInt(),
            p[2].toInt()
            );
        acm[i].series->setColor(QColor(col));
        if(acm[i].name_chart.contains("Давление",Qt::CaseInsensitive)){
            acm[i].series->setObjectName("bar");
        }
        if(acm[i].name_chart.contains("Температура",Qt::CaseInsensitive)){
            acm[i].series->setObjectName("temp");
        }
        /*if(acm[i].name_chart.contains("Давление",Qt::CaseInsensitive)){
            l_name_type->setStyleSheet("color: red");
            l_name->setStyleSheet("color: red");
        }
        if(acm[i].name_chart.contains("Температура",Qt::CaseInsensitive)){
            l_name_type->setStyleSheet("color: blue");
            l_name->setStyleSheet("color: blue");
        }*/
        p_hbox->addWidget(acm[i].check_box);
        p_hbox->addWidget(l_name_type);
        p_hbox->addWidget(l_name);
        p_hbox->addWidget(acm[i].label_data);
        p_hbox->addWidget(acm[i].label_delta);
        vbox_legend_->addLayout(p_hbox);
        data_base_.AddLabelSensor(l_name,l_name_type);
    }
    CreateMapLabel(data);
    CreateMapSeries(data);
}
void ChartView::PanelLegendEtalon(){
    QList<QLineSeries*> list_series;
    QHBoxLayout *hbox_time = new QHBoxLayout();
    QLabel *time = new QLabel();
    hbox_time->setAlignment(Qt::AlignLeft);
    hbox_time->addWidget(new QLabel("- Время:"));
    hbox_time->addWidget(time);
    vbox_legend_->addLayout(hbox_time);
    for(auto data : data_base_.GetDataSerEtalon()){
        QHBoxLayout *hbox = new QHBoxLayout();
        hbox->setAlignment(Qt::AlignLeft);
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
    QHBoxLayout *hbox_check_all = new QHBoxLayout();
    QCheckBox *check_all = new QCheckBox(" - Все графики");
    check_all->setCheckState(Qt::Checked);
    connect(check_all, &QCheckBox::toggled,this,[=](){
        if(check_all->isChecked()){
            for(auto& data : data_base_.GetDataSerACM()){
                QVector<Canal>& acm = data.vec_canal;
                for(Canal& check : acm){
                    check.check_box->setCheckState(Qt::Checked);
                }
            }
        } else {
            for(auto& data : data_base_.GetDataSerACM()){
                QVector<Canal>& acm = data.vec_canal;
                for(Canal& check : acm){
                    check.check_box->setCheckState(Qt::Unchecked);
                }
            }
        }
    });
    QCheckBox *check_delta = new QCheckBox(" - Погрешность");
    check_delta->setCheckState(Qt::Unchecked);
    connect(check_delta, &QCheckBox::toggled,this,&ChartView::FlagCalcDelta);
    hbox_check_all->addWidget(check_all);
    hbox_check_all->addWidget(check_delta);
    hbox_check_all->setAlignment(Qt::AlignLeft);
    vbox_legend_->addLayout(hbox_check_all);
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
void ChartView::CreateMapSeries(DataSeriesSensor& data){
    QVector<Canal>& acm = data.vec_canal;
    QList<QPointer<QLineSeries>> list_series;
    for(int i =0; i < acm.size();++i){
        list_series << acm[i].series;
    }
    for(int i =0; i < acm.size();++i){
        map_series_[acm[i].series->name()] = list_series;
    }
}
void ChartView::CreateMapLabel(DataSeriesSensor& data){
    QVector<Canal>& acm = data.vec_canal;
    for(int i =0; i < acm.size();++i){
        QPointer<QLabel> point = acm[i].label_data;
        map_data_label_[acm[i].series->name()] = point;
    }
}
void ChartView::ToogledFlagShiftSeries(){
    if(shift_series_){
        shift_series_ = false;
    } else {
        shift_series_ = true;
    }
}
void ChartView::ToogledFlagShiftCheckPoint(){
    if(shift_check_point_){
        shift_check_point_ = false;
    } else {
        shift_check_point_ = true;
    }
}
void ChartView::ToogledFlagLineInMouse(){
    if(data_in_time_){
        data_in_time_ = false;
        line_from_mouse_->hide();
    } else {
        data_in_time_ = true;
        line_from_mouse_max_ = axis_temp_etalon_->max();
        line_from_mouse_min_ = axis_temp_etalon_->min();
    }
}
void ChartView::MoveSeries(QLineSeries* series, qreal dx){
    QList<QPointF> points = series->points();
    for(QPointF& p : points){
        p.setX(p.x() + dx);
    }
    series->replace(points);
}
void ChartView::MoveCheckPoint(qreal point, qreal dx){
    qint64 t = static_cast<qint64>(point + dx);
    qint64 res = ((t + 500) / 1000) * 1000;
    QList<QPointF> points_temp = data_base_.GetDataSerEtalon()[0].series->points();
    QList<QPointF> points_bar = data_base_.GetDataSerEtalon()[1].series->points();
    for(int i =0 ; i<= points_temp.size()-1;++i){
        if(static_cast<qint64>(points_temp[i].x()) == res){
            data_base_.GetCheckPointTemp()[active_check_point_] = points_temp[i].y();
            data_base_.GetCheckPointBar()[active_check_point_] = points_bar[i].y();
            data_base_.GetCheckPoints()[active_check_point_] = QDateTime::fromMSecsSinceEpoch(points_temp[i].x());
            break;
        }
    }
    ReplaceCheckSeries();

}
void ChartView::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::RightButton){
        move_ = true;
        is_dragging_ = false;
        last_pos_mouse_ = event->pos();
    }
    if(event->button() == Qt::LeftButton && !shift_series_ && load_etalon_ && !shift_check_point_){
        last_pos_mouse_ = event->pos();
        band_.setGeometry(QRect(last_pos_mouse_, QSize()));
        band_.show();
    }
    if(event->button() == Qt::LeftButton && shift_check_point_){
        for(QAbstractSeries* abstract_series : chart_->series()){
            if(abstract_series->name() == "check_series"){
                QScatterSeries *series_check = qobject_cast<QScatterSeries*>(abstract_series);
                int index_point =0;
                for(QPointF& p : series_check->points()){
                    QPoint screen_pos = chart_->mapToPosition(p,series_check).toPoint();
                    if(QRect(screen_pos - QPoint(5,5),QSize(10,10)).contains(event->pos())){
                        active_check_point_ = index_point;
                        is_dragging_check_point_ = true;
                        active_check_series_ = series_check;
                        last_pos_mouse_ = screen_pos;
                        return;
                    }
                    index_point++;
                }
            }
        }
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
    if(is_dragging_check_point_){
        QPointF last = chart_->mapToValue(last_pos_mouse_);
        QPointF now = chart_->mapToValue(event->pos());
        qreal dx = now.x() - last.x();
        qreal point = data_base_.GetCheckPoints()[active_check_point_].toMSecsSinceEpoch();
        MoveCheckPoint(point, dx);
        last_pos_mouse_ = event->pos();
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
        SetLineFromMouse(mouse_pos);
        chart()->update();
        for(QAbstractSeries *series : chart_->series()){
            if(series != line_from_mouse_ && line_from_mouse_){
                QLineSeries *series_line = qobject_cast<QLineSeries*>(series);
                if(series_line){
                    const QVector<QPointF>& points = series_line->points();
                    int left= 0;
                    int right = points.size()-1;
                    QPointF found;
                    while(left <= right){
                        int mid = left + (right - left)/2;
                        QPoint screen_pos = chart_->mapToPosition(points[mid],series).toPoint();
                        if(screen_pos.x() == event->pos().x()){
                            found = points[mid];
                            break;
                        } else if (screen_pos.x() < event->pos().x() ){
                            left = mid + 1;
                        } else {
                            right = mid - 1;
                        }
                    }
                    if (left <= right) {
                        QDateTime time = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(found.x()));
                        QString time_str = time.toString("dd.MM.yyyy hh:mm:ss");
                        map_data_label_.value("time")->setText(time_str);
                        if(!map_data_label_.value(series->name()).isNull()){
                            map_data_label_.value(series->name())->setText(QString::number(found.y(),'f',2));
                            if(calc_delta_){
                                CalcDelta();
                            }
                        }

                    }
                }
            }
        }
    }
    QChartView::mouseMoveEvent(event);
}
void ChartView::FlagCalcDelta(){
    if(calc_delta_){
        calc_delta_ = false;
        QVector<DataSeriesSensor>& data_acm = data_base_.GetDataSerACM();
        for(DataSeriesSensor acm : data_acm){
            for(Canal& a : acm.vec_canal){
                a.label_delta->setText("");
            }
        }
    } else {
        calc_delta_ = true;
    }
}
void ChartView::CalcDelta(){
    QVector<DataSeriesSensor>& data_acm = data_base_.GetDataSerACM();
    QVector<DataSeriesEtalon>& data_etalon = data_base_.GetDataSerEtalon();
    QPointer<QLabel> label_bar = data_etalon[1].data_sensor;
    QPointer<QLabel> label_temp = data_etalon[0].data_sensor;
    for(DataSeriesSensor acm : data_acm){
        for(Canal& a : acm.vec_canal){
            if(a.name_type.contains("Давление",Qt::CaseInsensitive)){
                a.label_delta->setText(" (" + QString::number(a.label_data->text().toDouble() - label_bar->text().toDouble(),'f',2) + ")");
            } else {
                a.label_delta->setText(" (" + QString::number(a.label_data->text().toDouble() - label_temp->text().toDouble(),'f',2) + ")");
            }
        }
    }

}
void ChartView::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::RightButton){
        is_dragging_ = false;
    }
    if(event->button() == Qt::LeftButton){
        is_dragging_check_point_ = false;

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
            if(p2.x()-p1.x() > 10){
                ZoomChart(rect);
            }
        }
        line_from_mouse_max_ = axis_temp_etalon_->max();
        line_from_mouse_min_ = axis_temp_etalon_->min();
    }

    chart()->update();
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
        std::pair<QDateTime,QDateTime> time = data_base_.GetDefaultAxisX();
        axis_time_->setRange(time.first,time.second);
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
void ChartView::ZeroZoom(){
    if(box_zoom_.empty()){
        std::pair<QDateTime,QDateTime> time = data_base_.GetDefaultAxisX();
        axis_time_->setRange(time.first,time.second);
        return;
    }
    BoxZoom box = box_zoom_[0];
    axis_bar_->setRange(box.bar_min_acm_,box.bar_max_acm_);
    axis_temp_->setRange(box.temp_min_acm_, box.temp_max_acm_);
    axis_time_->setRange(box.axis_min,box.axis_max);
    axis_bar_etalon_->setRange(box.bar_min_etalon_,box.bar_max_etalon_);
    axis_temp_etalon_->setRange(box.temp_min_etalon_,box.temp_max_etalon_);
    box_zoom_.clear();
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
                    if(series->objectName() == "bar"){
                        if(!first_bar_acm){
                            bar_max_acm_ = y;
                            bar_min_acm_ = y;
                            first_bar_acm= true;
                        }
                        bar_max_acm_ = qMax(bar_max_acm_,y);
                        bar_min_acm_ = qMin(bar_min_acm_,y);
                    }
                    if(series->objectName() == "temp"){
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
    if(auto_zoom_){
        axis_bar_->setRange(bar_min_etalon_,bar_max_etalon_ + bar_max_etalon_*0.1);
        axis_temp_->setRange(temp_min_etalon_,temp_max_etalon_ + temp_max_etalon_*0.1);
    } else {
        axis_bar_->setRange(bar_min_acm_,bar_max_acm_ + bar_max_acm_*0.05);
        axis_temp_->setRange(temp_min_acm_,temp_max_acm_ + temp_max_acm_*0.05);
    }

}
void ChartView::SetLineFromMouse(QPointF point){
    double mouse_x = point.x();
    line_from_mouse_->clear();
    *line_from_mouse_ << QPointF(mouse_x, line_from_mouse_min_) << QPointF(mouse_x, line_from_mouse_max_);
}
void ChartView::ZoomOn(){
    load_etalon_ = true;
}
void ChartView::ClearPanelLegend(){
    while(QLayoutItem *item =vbox_legend_->itemAt(0)){
        if(QWidget *wid = item->widget()){
            delete wid;
        }
        if(QLayout *l = item->layout()){
            while(QLayoutItem *it = l->itemAt(0)){
                if(QWidget *wid = it->widget()){
                    delete wid;
                }
            }
            delete l;
        }
    }
}
void ChartView::ReplaceCheckSeries(){
    QVector<QPointF> bar;
    QVector<QPointF> temp;
    for(int i = 0; i<= data_base_.GetCheckPoints().size()-1;++i){
        bar.append(QPointF(data_base_.GetCheckPoints()[i].toMSecsSinceEpoch(),data_base_.GetCheckPointBar()[i]));
        temp.append(QPointF(data_base_.GetCheckPoints()[i].toMSecsSinceEpoch(),data_base_.GetCheckPointTemp()[i]));
    }
    data_base_.GetDataSerEtalon()[0].point_series->replace(temp);
    data_base_.GetDataSerEtalon()[1].point_series->replace(bar);
}
void ChartView::ReBuildPointSeries(){
    QVector<QPointF> points_bar = data_base_.GetDataSerEtalon()[1].point_series->points().toVector();
    std::sort(points_bar.begin(),points_bar.end(), [](const QPointF& a, const QPointF& b){
        return a.x() < b.x();
    });
    QVector<QPointF> points_temp = data_base_.GetDataSerEtalon()[0].point_series->points().toVector();
    std::sort(points_temp.begin(),points_temp.end(), [](const QPointF& a, const QPointF& b){
        return a.x() < b.x();
    });
    for(int i =0; i <= data_base_.GetCheckPointTemp().size()-1; ++i){
        data_base_.GetCheckPoints()[i] = QDateTime::fromMSecsSinceEpoch(points_temp[i].x());
        data_base_.GetCheckPointTemp()[i] = points_temp[i].y();
        data_base_.GetCheckPointBar()[i] = points_bar[i].y();
    }
}
void  ChartView::AutoZoom(){
    if(auto_zoom_){
        auto_zoom_ = false;
    } else {
        auto_zoom_ = true;
    }
}
