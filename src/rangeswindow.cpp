#include "rangeswindow.h"
#include "data_base.h"
#include "chartview.h"
#include "util.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QTableView>
#include <QStandardItemModel>
#include <QTimeEdit>
#include <QHeaderView>
#include <QLineSeries>
#include <algorithm>

RangesWindow::RangesWindow(DataBase& data_base, ChartView* chart_view, QWidget* parent)
    : QWidget(parent), data_base_(data_base), chart_view_(chart_view){
    setWindowTitle("Контрольные диапазоны");
    setWindowFlags(Qt::WindowStaysOnTopHint);
    table_ = new QTableView();
    model_ = new QStandardItemModel();

    QGroupBox *group_new = new QGroupBox("Добавление диапазона");
    QVBoxLayout *vbox_new = new QVBoxLayout();
    QHBoxLayout *hbox_new = new QHBoxLayout();
    hbox_new->setAlignment(Qt::AlignLeft);
    QTimeEdit *s_t1 = new QTimeEdit();
    s_t1->setDisplayFormat("hh:mm:ss");
    QTimeEdit *s_t2 = new QTimeEdit();
    s_t2->setDisplayFormat("hh:mm:ss");
    QPushButton *btn_create = new QPushButton("Добавить");
    hbox_new->addWidget(new QLabel("t1"));
    hbox_new->addWidget(s_t1);
    hbox_new->addWidget(new QLabel("t2"));
    hbox_new->addWidget(s_t2);
    vbox_new->addLayout(hbox_new);
    vbox_new->addWidget(btn_create);
    QCheckBox* show_cb = new QCheckBox("Показывать на графике");
    show_cb->setChecked(chart_view_->IsRangesVisible());
    connect(show_cb, &QCheckBox::toggled, this, [this](bool on){ chart_view_->SetRangesVisible(on); });
    connect(chart_view_, &ChartView::RangesVisibilityChanged, show_cb, &QCheckBox::setChecked);
    vbox_new->addWidget(show_cb);
    group_new->setLayout(vbox_new);
    connect(btn_create, &QPushButton::clicked, this, [=](){
        AddRangeAt(s_t1->time(), s_t2->time());
    });

    QPushButton *btn_delete = new QPushButton("Удалить");
    QPushButton *btn_rebuild = new QPushButton("Обновить");
    connect(btn_delete, &QPushButton::clicked, this, &RangesWindow::DeleteSelected);
    connect(btn_rebuild, &QPushButton::clicked, this, &RangesWindow::Refresh);

    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setModel(model_);
    table_->setSortingEnabled(true);

    QVBoxLayout *vbox = new QVBoxLayout();
    QHBoxLayout *hbox_btn = new QHBoxLayout();
    hbox_btn->setAlignment(Qt::AlignLeft);
    hbox_btn->addWidget(btn_delete);
    hbox_btn->addWidget(btn_rebuild);
    vbox->addWidget(group_new);
    vbox->addWidget(table_);
    vbox->addLayout(hbox_btn);
    setLayout(vbox);
    resize(460, 240);
}

void RangesWindow::AddRange(QTime t1, QTime t2){
    if(t1 > t2){
        std::swap(t1, t2);
    }
    QVector<DataSeriesEtalon>& et = data_base_.GetDataSerEtalon();
    if(et.size() < 2){
        return;
    }
    double sum_temp = 0; int n_temp = 0;
    double sum_bar = 0;  int n_bar = 0;
    qint64 min_ms = -1, max_ms = -1;
    auto accumulate = [&](QLineSeries* series, double& sum, int& count){
        if(!series){
            return;
        }
        for(const QPointF& p : series->points()){
            qint64 ms = RoundToSec(static_cast<qint64>(p.x()));
            QTime tt = QDateTime::fromMSecsSinceEpoch(ms).time();
            if(tt >= t1 && tt <= t2){
                sum += p.y();
                count++;
                if(min_ms < 0 || ms < min_ms){ min_ms = ms; }
                if(max_ms < 0 || ms > max_ms){ max_ms = ms; }
            }
        }
    };
    accumulate(et[0].series, sum_temp, n_temp);   // [0] — температура (ЛТ300)
    accumulate(et[1].series, sum_bar, n_bar);     // [1] — давление (ДМ5002М)
    if(n_temp == 0 && n_bar == 0){
        return;   // в отрезок не попала ни одна точка
    }
    CheckRange range;
    range.t_start = QDateTime::fromMSecsSinceEpoch(min_ms);
    range.t_end = QDateTime::fromMSecsSinceEpoch(max_ms);
    range.t_mid = QDateTime::fromMSecsSinceEpoch((min_ms + max_ms) / 2);
    range.avg_temp = n_temp ? sum_temp / n_temp : 0;
    range.avg_bar = n_bar ? sum_bar / n_bar : 0;
    data_base_.GetCheckRanges().push_back(range);
}

void RangesWindow::AddRangeAt(QTime t1, QTime t2){
    AddRange(t1, t2);
    chart_view_->RefreshOverlay();
    Refresh();
}

void RangesWindow::DeleteSelected(){
    if(table_->selectionModel()->hasSelection()){
        QModelIndex index = table_->currentIndex();
        QVector<CheckRange>& ranges = data_base_.GetCheckRanges();
        if(index.row() >= 0 && index.row() < ranges.size()){
            ranges.removeAt(index.row());
            Refresh();
            chart_view_->RefreshOverlay();
        }
    }
}

void RangesWindow::Refresh(){
    model_->clear();
    QVector<CheckRange>& ranges = data_base_.GetCheckRanges();
    for(int i = 0; i < ranges.size(); ++i){
        QStandardItem *t_start = new QStandardItem(ranges[i].t_start.time().toString());
        t_start->setTextAlignment(Qt::AlignCenter);
        model_->setItem(i, 0, t_start);
        QStandardItem *t_end = new QStandardItem(ranges[i].t_end.time().toString());
        t_end->setTextAlignment(Qt::AlignCenter);
        model_->setItem(i, 1, t_end);
        QStandardItem *avg_temp = new QStandardItem(QString::number(ranges[i].avg_temp, 'f', 2));
        avg_temp->setTextAlignment(Qt::AlignCenter);
        model_->setItem(i, 2, avg_temp);
        QStandardItem *avg_bar = new QStandardItem(QString::number(ranges[i].avg_bar, 'f', 2));
        avg_bar->setTextAlignment(Qt::AlignCenter);
        model_->setItem(i, 3, avg_bar);
    }
    model_->setHeaderData(0, Qt::Horizontal, "Начало");
    model_->setHeaderData(1, Qt::Horizontal, "Конец");
    model_->setHeaderData(2, Qt::Horizontal, "Сред. темп.");
    model_->setHeaderData(3, Qt::Horizontal, "Сред. давл.");
}
