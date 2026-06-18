#include "checkpointswindow.h"
#include "data_base.h"
#include "chartview.h"
#include "util.h"
#include <QChart>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>
#include <QTableView>
#include <QStandardItemModel>
#include <QTimeEdit>
#include <QHeaderView>

CheckPointsWindow::CheckPointsWindow(DataBase& data_base, ChartView* chart_view, QWidget* parent)
    : QWidget(parent), data_base_(data_base), chart_view_(chart_view){
    setWindowTitle("Контролтные точки");
    fix_table_ = new QTableView();
    fix_model_ = new QStandardItemModel();
    QGroupBox *create_new_point = new QGroupBox("Добавление КТ");
    QVBoxLayout *vbox_new_point = new QVBoxLayout();
    QHBoxLayout *hbox_new_point = new QHBoxLayout();
    hbox_new_point->setAlignment(Qt::AlignLeft);
    vbox_new_point->setAlignment(Qt::AlignLeft);
    QPushButton *btn_create = new QPushButton("Добавить");
    QTimeEdit *s_hour = new QTimeEdit();
    s_hour->setDisplayFormat("hh:mm:ss");
    hbox_new_point->addWidget(s_hour);
    vbox_new_point->addLayout(hbox_new_point);
    vbox_new_point->addWidget(btn_create);
    QCheckBox* show_cb = new QCheckBox("Показывать на графике");
    show_cb->setChecked(chart_view_->IsCheckPointsVisible());
    connect(show_cb, &QCheckBox::toggled, this, [this](bool on){ chart_view_->SetCheckPointsVisible(on); });
    connect(chart_view_, &ChartView::CheckPointsVisibilityChanged, show_cb, &QCheckBox::setChecked);
    vbox_new_point->addWidget(show_cb);
    create_new_point->setLayout(vbox_new_point);
    connect(btn_create, &QPushButton::clicked,this, [=](){
        AddCheckPointAt(s_hour->time());
    });
    QPushButton *btn_delete = new QPushButton("Удалить КТ");
    QPushButton *btn_rebuild = new QPushButton("Обновить");
    QHBoxLayout *hbox = new QHBoxLayout();
    QVBoxLayout *vbox = new QVBoxLayout();
    fix_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    fix_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(btn_delete,&QPushButton::clicked, this,&CheckPointsWindow::DeleteSelected);
    connect(btn_rebuild,&QPushButton::clicked, this,&CheckPointsWindow::Refresh);
    fix_table_->setWindowTitle("Таблица контрольных точек");
    setWindowFlags(Qt::WindowStaysOnTopHint);
    fix_table_->setWindowOpacity(0.9);
    fix_table_->setModel(fix_model_);
    fix_table_->setSortingEnabled(true);
    vbox->addWidget(create_new_point);
    vbox->addWidget(fix_table_);
    hbox->setAlignment(Qt::AlignLeft);
    hbox->addWidget(btn_delete);
    hbox->addWidget(btn_rebuild);
    vbox->addLayout(hbox);
    setLayout(vbox);
    resize(380,200);
}

void CheckPointsWindow::AddCheckPointAt(QTime time_read){
    AddCheckPoint(time_read);
    chart_view_->ReBuildPointSeries();
    Refresh();
}

void CheckPointsWindow::DeleteCheckPointAtTime(QTime time){
    QVector<QDateTime>& times = data_base_.GetCheckPoints();
    for(int i = 0; i < times.size(); ++i){
        if(times[i].time() == time){
            data_base_.GetCheckPointTemp().removeAt(i);
            data_base_.GetCheckPointBar().removeAt(i);
            times.removeAt(i);
            Refresh();
            chart_view_->ReplaceCheckSeries();
            chart_view_->ReBuildPointSeries();
            break;
        }
    }
}

void CheckPointsWindow::AddCheckPoint(QTime time_read){
    double temp = 0.0;
    double bar = 0.0;
    QDateTime date_study;
    QVector<DataSeriesEtalon>& vec = data_base_.GetDataSerEtalon();
    for(DataSeriesEtalon& v : vec){
        if(v.series){
            for(QPointF point : v.series->points()){
                qint64 t = static_cast<qint64>(point.x());
                qint64 res = RoundToSec(t);
                QDateTime date = QDateTime::fromMSecsSinceEpoch(res);
                QTime time = date.time();
                if(time_read == time){
                    v.point_series->append(point);
                    if(v.name_series == "ЛТ300"){
                        temp = point.y();
                    }
                    if(v.name_series == "ДМ5002М"){
                        bar = point.y();
                    }
                    date_study = date;
                    break;
                }
            }
        }
    }
    QVector<QDateTime>& dat = data_base_.GetCheckPoints();
    QVector<double>& v_bar = data_base_.GetCheckPointBar();
    QVector<double>& v_temp = data_base_.GetCheckPointTemp();
    if(!data_base_.GetCheckPoints().isEmpty()){
        for(int i = 0;i< dat.size();++i){
            if(time_read < dat[i].time()){
                QDateTime check(dat[0].date(),time_read);
                dat.insert(dat.begin() + i,check);
                v_bar.insert(v_bar.begin() + i, bar);
                v_temp.insert(v_temp.begin() + i, temp);
                break;
            }
            if(i+1 == dat.size()){
                QDateTime check(dat[0].date(),time_read);
                dat.insert(dat.end(),check);
                v_bar.insert(v_bar.end(), bar);
                v_temp.insert(v_temp.end(), temp);
                break;
            }
        }
    } else {
        QDateTime check(date_study.date(),time_read);
        dat.push_back(check);
        v_bar.push_back(bar);
        v_temp.push_back(temp);
    }
}

void CheckPointsWindow::DeleteSelected(){
    if(fix_table_->selectionModel()->hasSelection()){
        QModelIndex index = fix_table_->currentIndex();
        auto it_time = data_base_.GetCheckPoints().begin();
        auto it_bar = data_base_.GetCheckPointBar().begin();
        auto it_temp = data_base_.GetCheckPointTemp().begin();
        data_base_.GetCheckPointTemp().erase(it_temp+index.row());
        data_base_.GetCheckPointBar().erase(it_bar+index.row());
        data_base_.GetCheckPoints().erase(it_time+index.row());
        Refresh();
        chart_view_->ReplaceCheckSeries();
        chart_view_->ReBuildPointSeries();
    }
}

void CheckPointsWindow::Refresh(){
    fix_model_->clear();
    if(!data_base_.GetCheckPoints().isEmpty()){
        for(int i =0; i <= data_base_.GetCheckPoints().size()-1; ++i){
            QStandardItem *time = new QStandardItem(data_base_.GetCheckPoints()[i].time().toString());
            time->setTextAlignment(Qt::AlignCenter);
            fix_model_->setItem(i,0,time);
            QString t_bar = QString::number(data_base_.GetCheckPointBar()[i]);
            QStandardItem *bar = new QStandardItem(t_bar);
            bar->setTextAlignment(Qt::AlignCenter);
            fix_model_->setItem(i,1,bar);
            QString t_temp = QString::number(data_base_.GetCheckPointTemp()[i]);
            QStandardItem *temp = new QStandardItem(t_temp);
            temp->setTextAlignment(Qt::AlignCenter);
            fix_model_->setItem(i,2,temp);
        }
    }
    fix_model_->setHeaderData(0,Qt::Horizontal,"Время");
    fix_model_->setHeaderData(1,Qt::Horizontal,"Давление");
    fix_model_->setHeaderData(2,Qt::Horizontal,"Температура");
    chart_view_->GetChart()->update();
}
