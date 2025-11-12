#include "error_table.h"

#include <QPushButton>

ErrorTable::ErrorTable(DataBase& data_base, CreateRaport& create_raport, QWidget *parent)
    : QWidget(parent), data_base_(data_base),create_raport_(create_raport)

{
    QVBoxLayout *vbox = new QVBoxLayout();
    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setAlignment(Qt::AlignLeft);
    QPushButton *btn = new QPushButton("Обновить");
    connect(btn, &QPushButton::clicked, this, &ErrorTable::FillTable);
    QPushButton *btn_rap = new QPushButton("Создать отчеты");
    connect(btn_rap, &QPushButton::clicked, this, &ErrorTable::CreateRapor);
    table_view_ = new QTableView();
    setWindowTitle("Таблица погрешностей");
    model_ = new QStandardItemModel();
    table_view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_view_->setModel(model_);
    hbox->addWidget(btn);
    hbox->addWidget(btn_rap);
    vbox->addLayout(hbox,1);
    vbox->addWidget(table_view_,9);
    setLayout(vbox);
    resize(500,600);
}

void ErrorTable::FillTable(){
    model_->clear();
    QVector<QDateTime> times = data_base_.GetCheckPoints();
    QVector<double> bar_data = data_base_.GetCheckPointBar();
    QVector<double> temp_data = data_base_.GetCheckPointTemp();
    QVector<DataSeriesACM> data_acm = data_base_.GetDataSerACM();
    for(int i =0; i <= data_base_.GetCheckPoints().size()-1; ++i){
        QStandardItem *time = new QStandardItem(times[i].time().toString());
        QFont font_t = time->font();
        font_t.setBold(true);
        time->setFont(font_t);
        time->setTextAlignment(Qt::AlignCenter);
        model_->setItem(i,0,time);
        QString t_bar = QString::number(bar_data[i], 'f',2);
        QStandardItem *bar = new QStandardItem(t_bar);
        QFont font = bar->font();
        font.setBold(true);
        bar->setFont(font);
        bar->setTextAlignment(Qt::AlignCenter);
        model_->setItem(i,1,bar);
        QString t_temp = QString::number(temp_data[i], 'f',2);
        QStandardItem *temp = new QStandardItem(t_temp);
        QFont font_temp = time->font();
        font_t.setBold(true);
        temp->setFont(font_temp);
        temp->setTextAlignment(Qt::AlignCenter);
        model_->setItem(i,2,temp);
        if(i % 2 == 0){
            time->setBackground(QColor(230,230,230));
            bar->setBackground(QColor(230,230,230));
            temp->setBackground(QColor(230,230,230));
        }
    }
    for(DataSeriesACM acm : data_acm){
        AnalisingSeries(acm);
    }

    model_->setHeaderData(0,Qt::Horizontal,"Время");
    model_->setHeaderData(0, Qt::Horizontal, QFont("Arial", 10, QFont::Bold), Qt::FontRole);
    model_->setHeaderData(1,Qt::Horizontal,"Pэт");
    model_->setHeaderData(1, Qt::Horizontal, QFont("Arial", 10, QFont::Bold), Qt::FontRole);
    model_->setHeaderData(2,Qt::Horizontal,"Тэт");
    model_->setHeaderData(2, Qt::Horizontal, QFont("Arial", 10, QFont::Bold), Qt::FontRole);
    table_view_->resizeColumnsToContents();

}


void ErrorTable::AnalisingSeries(DataSeriesACM data){
    QVector<ACM>& acm = data.vec_acm;
    QPointer<QLineSeries> series_bar;
    QPointer<QLineSeries> series_temp;
    QString name = data.name_sensor;
    QVector<double> del_temp;
    QVector<double> vol_temp;
    QVector<double> del_bar;
    QVector<double> vol_bar;
    double delta;
    double value;
    for(ACM& a :acm){
        if(a.name_chart.contains("Давление",Qt::CaseInsensitive)){
            series_bar = a.series;
        }
        if(a.name_chart.contains("Температура",Qt::CaseInsensitive)){
            series_temp = a.series;
        }
    }
    const QVector<QDateTime> vec = data_base_.GetCheckPoints();
    int count = 0;
    int row = 0;
    int column = model_->columnCount();
    if(series_bar){
        for(QPointF& point : series_bar->points()){
            if(count == data_base_.GetCheckPoints().size()){
                break;
            }
            qint64 t = static_cast<qint64>(point.x());
            qint64 res = ((t + 500) / 1000) * 1000;
            QDateTime time = QDateTime::fromMSecsSinceEpoch(res);
            if(time == vec[count]){
                count++;
                value = model_->data(model_->index(row,1)).toDouble();
                delta = static_cast<double>(point.y()) - value;
                vol_bar.push_back(static_cast<double>(point.y()));
                del_bar.push_back(delta);
                QStandardItem *it_del = new QStandardItem(QString::number(delta, 'f',2));
                QStandardItem *it = new QStandardItem(QString::number(point.y(), 'f',2));
                if(row % 2 == 1 && column % 2 == 0 ){
                    it->setBackground(QColor(240,240,240));
                    it_del->setBackground(QColor(240,240,240));
                };
                if(row % 2 == 0){
                    it->setBackground(QColor(220,220,220));
                    it_del->setBackground(QColor(220,220,220));
                };
                if(delta > 1.5 || delta < -1.5){
                    it_del->setBackground(Qt::yellow);
                }
                it_del->setTextAlignment(Qt::AlignCenter);
                it->setTextAlignment(Qt::AlignCenter);
                if(model_){
                    model_->setItem(row,column,it);
                    model_->setItem(row,column+1,it_del);
                }
                row++;
            }
        }
        model_->setHeaderData(column+1,Qt::Horizontal,"dP абс.");
        model_->setHeaderData(column+1, Qt::Horizontal, QFont("Arial", 10, QFont::Bold), Qt::FontRole);
        model_->setHeaderData(column,Qt::Horizontal,"Pизм " + name);
        model_->setHeaderData(column, Qt::Horizontal, QFont("Arial", 10, QFont::Bold), Qt::FontRole);
        column += 2;
        count = 0;
        row = 0;
    }
    if(series_temp){
        for(QPointF& point : series_temp->points()){
            if(count == data_base_.GetCheckPoints().size()){
                break;
            }
            qint64 t = static_cast<qint64>(point.x());
            qint64 res = ((t + 500) / 1000) * 1000;
            QDateTime time = QDateTime::fromMSecsSinceEpoch(res);
            if(time == vec[count]){
                count++;
                value = model_->data(model_->index(row,2)).toDouble();
                delta = static_cast<double>(point.y()) - value;
                vol_temp.push_back(static_cast<double>(point.y()));
                del_temp.push_back(delta);
                QStandardItem *it_del = new QStandardItem(QString::number(delta, 'f',2));
                QStandardItem *it = new QStandardItem(QString::number(point.y(), 'f',2));
                if(row % 2 == 1 && column % 2 == 1 ){
                    it->setBackground(QColor(240,240,240));
                    it_del->setBackground(QColor(240,240,240));
                };
                if(row % 2 == 0){
                    it->setBackground(QColor(220,220,220));
                    it_del->setBackground(QColor(220,220,220));
                };
                if(delta > 1.5 || delta < -1.5){
                    it_del->setBackground(Qt::yellow);
                }
                it_del->setTextAlignment(Qt::AlignCenter);
                it->setTextAlignment(Qt::AlignCenter);
                if(model_){
                    model_->setItem(row,column,it);
                    model_->setItem(row,column+1,it_del);
                }
                row++;
            }
        }
        model_->setHeaderData(column+1,Qt::Horizontal,"dT абс.");
        model_->setHeaderData(column+1, Qt::Horizontal, QFont("Arial", 10, QFont::Bold), Qt::FontRole);
        model_->setHeaderData(column,Qt::Horizontal,"Tизм " + name);
        model_->setHeaderData(column, Qt::Horizontal, QFont("Arial", 10, QFont::Bold), Qt::FontRole);
        column += 2;
    }
    data_base_.AddDeltaVolData(name,del_bar,vol_bar,del_temp,vol_temp);
}
void ErrorTable::CreateRapor(){
    create_raport_.CreateAllDeltaDoc();
}
