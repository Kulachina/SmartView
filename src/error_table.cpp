#include "error_table.h"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QTextStream>
#include <QApplication>
#include <QStandardPaths>
#include <QMessageBox>
#include <QFileDialog>

ErrorTable::ErrorTable(DataBase& data_base, CreateRaport& create_raport, QWidget *parent)
    : QWidget(parent), data_base_(data_base),create_raport_(create_raport)

{
    QVBoxLayout *vbox = new QVBoxLayout();
    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setAlignment(Qt::AlignLeft);
    QPushButton *btn = new QPushButton("Обновить");
    connect(btn, &QPushButton::clicked, this, &ErrorTable::FillTable);
    QPushButton *btn_rap = new QPushButton("Создать отчеты");
    connect(btn_rap, &QPushButton::clicked, this, &ErrorTable::CreateAllDeltaDoc);
    tab_ = new QTabWidget();
    setWindowTitle("Таблица погрешностей");
    hbox->addWidget(btn);
    hbox->addWidget(btn_rap);
    vbox->addLayout(hbox,1);
    vbox->addWidget(tab_,9);
    setLayout(vbox);
    resize(500,600);
}

void ErrorTable::FillTable(){
    QVector<DataSeriesSensor> data_acm = data_base_.GetDataSerACM();
    tab_->clear();
    DeleteTable();
    for(DataSeriesSensor& acm : data_acm){
        QTableWidget *table = new QTableWidget();
        table->setObjectName(acm.name_sensor);
        ptr_table.push_back(table);
        table->setColumnCount(7);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        FillEtalon(table);
        AnalisingSeries(acm,table);
        table->resizeColumnsToContents();
        tab_->addTab(table, acm.name_sensor);
    }
}

void ErrorTable::FillEtalon(QTableWidget *table){
    QVector<QDateTime> times = data_base_.GetCheckPoints();
    QVector<double> bar_data = data_base_.GetCheckPointBar();
    QVector<double> temp_data = data_base_.GetCheckPointTemp();
    table->setRowCount(data_base_.GetCheckPoints().size());
    for(int i =0; i <= data_base_.GetCheckPoints().size()-1; ++i){
        QTableWidgetItem *time = new QTableWidgetItem(times[i].time().toString());
        QFont font_t = time->font();
        font_t.setBold(true);
        time->setFont(font_t);
        time->setTextAlignment(Qt::AlignCenter);
        table->setItem(i,0,time);
        QString t_bar = QString::number(bar_data[i], 'f',2);
        QTableWidgetItem *bar = new QTableWidgetItem(t_bar);
        QFont font = bar->font();
        font.setBold(true);
        bar->setFont(font);
        bar->setTextAlignment(Qt::AlignCenter);
        table->setItem(i,1,bar);
        QString t_temp = QString::number(temp_data[i], 'f',2);
        QTableWidgetItem *temp = new QTableWidgetItem(t_temp);
        QFont font_temp = time->font();
        font_t.setBold(true);
        temp->setFont(font_temp);
        temp->setTextAlignment(Qt::AlignCenter);
        table->setItem(i,2,temp);
    }
    table->setHorizontalHeaderItem(0,new QTableWidgetItem("Время"));
    table->setHorizontalHeaderItem(1,new QTableWidgetItem("Pэт"));
    table->setHorizontalHeaderItem(2,new QTableWidgetItem("Tэт"));
}

void ErrorTable::AnalisingSeries(DataSeriesSensor& data,QTableWidget *table){
    QVector<Canal>& acm = data.vec_canal;
    QPointer<QLineSeries> series_bar;
    QPointer<QLineSeries> series_temp;
    QString name = data.name_sensor;
    QString type_error_temp;
    QString type_error_bar;
    QVector<double> del_temp;
    QVector<double> vol_temp;
    QVector<double> del_bar;
    QVector<double> vol_bar;
    int type_er_bar;
    int type_er_temp;
    double accept_min_bar;
    double accept_max_bar;
    double accept_min_temp;
    double accept_max_temp;
    int duration_temp;
    int duration_bar;
    double delta;
    double value;
    for(Canal& a :acm){
        if(a.name_canal.contains("Давление",Qt::CaseInsensitive)){
            series_bar = a.series;
            if(a.type_error == 1){
                type_error_bar = "dP абс.";
                type_er_bar =1;
            }
            if(a.type_error == 2){
                type_error_bar = "dP отн.";
                type_er_bar = 2;
            }
            if(a.type_error == 3){
                type_error_bar = "dP прв.";
                type_er_bar = 3;
                duration_bar = a.duration_error_max - a.duration_error_min;
            }
            accept_max_bar = a.accept_max;
            accept_min_bar = a.accept_min;
        }
        if(a.name_canal.contains("Температура",Qt::CaseInsensitive)){
            series_temp = a.series;
            if(a.type_error == 1){
                type_error_temp = "dT абс.";
                type_er_temp = 1;
            }
            if(a.type_error == 2){
                type_error_temp = "dT отн.";
                type_er_temp = 2;
            }
            if(a.type_error == 3){
                type_error_temp = "dT прв.";
                type_er_temp = 3;
                duration_temp = a.duration_error_max - a.duration_error_min;
            }
            accept_max_temp = a.accept_max;
            accept_min_temp = a.accept_min;
        }
    }
    const QVector<QDateTime>& vec = data_base_.GetCheckPoints();
    int count = 0;
    int row = 0;
    if(series_bar){
        for(QPointF& point : series_bar->points()){
            if(count == data_base_.GetCheckPoints().size()){
                break;
            }
            qint64 t = static_cast<qint64>(point.x());
            qint64 res = ((t + 500) / 1000) * 1000;
            QDateTime time = QDateTime::fromMSecsSinceEpoch(res);
            QDateTime dt = vec[count];
            dt = dt.addMSecs(-dt.time().msec());
            if(time == dt){
                count++;
                value = table->item(row,1)->text().toDouble();

                delta = CalcError(type_er_bar,value,point.y(),duration_bar);
                //delta = static_cast<double>(point.y()) - value;
                vol_bar.push_back(static_cast<double>(point.y()));
                del_bar.push_back(delta);
                QTableWidgetItem *it_del = new QTableWidgetItem(QString::number(delta, 'f',2));
                QTableWidgetItem *it = new QTableWidgetItem(QString::number(point.y(), 'f',2));
                if(delta > accept_max_bar || delta < accept_min_bar){
                    it_del->setBackground(Qt::yellow);
                }
                it_del->setTextAlignment(Qt::AlignCenter);
                it->setTextAlignment(Qt::AlignCenter);
                if(table){
                    table->setItem(row,3,it);
                    table->setItem(row,4,it_del);
                }
                row++;
            }
        }
        table->setHorizontalHeaderItem(3,new QTableWidgetItem("Pизм"));
        table->setHorizontalHeaderItem(4,new QTableWidgetItem(type_error_bar));
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
            QDateTime dt = vec[count];
            dt = dt.addMSecs(-dt.time().msec());
            if(time == dt){
                count++;
                value = table->item(row,2)->text().toDouble();
                delta = CalcError(type_er_temp,value,point.y(),duration_temp);
                //delta = static_cast<double>(point.y()) - value;
                vol_bar.push_back(static_cast<double>(point.y()));
                del_bar.push_back(delta);
                QTableWidgetItem *it_del = new QTableWidgetItem(QString::number(delta, 'f',2));
                QTableWidgetItem *it = new QTableWidgetItem(QString::number(point.y(), 'f',2));
                if(delta > accept_max_temp || delta < accept_min_temp){
                    it_del->setBackground(Qt::yellow);
                }
                it_del->setTextAlignment(Qt::AlignCenter);
                it->setTextAlignment(Qt::AlignCenter);
                if(table){
                    table->setItem(row,5,it);
                    table->setItem(row,6,it_del);
                }
                row++;
            }
        }
        table->setHorizontalHeaderItem(5,new QTableWidgetItem("Tизм"));
        table->setHorizontalHeaderItem(6,new QTableWidgetItem(type_error_temp));
    }
    data_base_.AddDeltaVolData(name,del_bar,vol_bar,del_temp,vol_temp);
}
void ErrorTable::CreateAllDeltaDoc(){
    QString path = QApplication::applicationDirPath();
    QString dir_name = QFileDialog::getExistingDirectory(nullptr, "Сохранить контрольные точки", path);
    QVector<DataSeriesSensor>& data = data_base_.GetDataSerACM();
    for(DataSeriesSensor& acm : data){
        QString file_data ="/" + acm.name_sensor.trimmed() + "_Погрешность.csv";
        QString file_name = dir_name +  file_data  ;
        if(file_name.isEmpty()){
            return;
        }
        QTableWidget* table = tab_->findChild<QTableWidget*>(acm.name_sensor);
        if(table){
            CreateDeltaDoc(table, file_name, acm.name_sensor.trimmed());
        };
    }
}
void ErrorTable::CreateDeltaDoc(QTableWidget* table, QString file_name,QString name_sensor){
    QFile raport(file_name);
    if(!raport.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::warning(nullptr, "Ошибка","Неудалось открыть файл для записи");
        return;
    }
    raport.write("\xEF\xBB\xBF");
    QTextStream out(&raport);
    out.setEncoding(QStringConverter::Utf8);
    out <<  name_sensor << "\n";
    if(table){
        for (int col = 0; col < table->columnCount(); ++col) {
            QString text = table->model()->headerData(col, Qt::Horizontal).toString();
            out << text << ";";
            //out << QString("%1").arg(text,10);
            if(col == table->columnCount()-1){
                out <<"\n";
            }
        }
        for(int row = 0; row < table->rowCount() ; ++row){
            for(int column = 0;column < table->columnCount();column++){
                QString word = table->item(row,column)->text();
                if(column >= 1){
                    word.replace('.',',');
                }
                out << word +";";
                //out << QString("%1").arg(word,10);
                if(column == table->columnCount()-1){
                    out <<"\n";
                }
            }
        }
    }
    raport.close();
}
double ErrorTable::CalcError(int type, double etalon, double data, int duration){
    if(type == 1){
        return data - etalon;
    }
    if(type == 2){
        double abs = data - etalon;
        double otn = (abs/etalon) * 100;
        if(otn < 0){
            otn = otn * -1;
        }
        return otn;
    }
    if(type == 3){
        double abs = data - etalon;
        double prv = (abs/duration) * 100;
        if(prv < 0){
            prv = prv * -1;
        }
        return prv;
    }
    return 999.25;
}

void ErrorTable::DeleteTable(){
    if(!ptr_table.isEmpty()){
        for(QTableWidget* ptr :ptr_table){
            delete ptr;
        }
        ptr_table.clear();
    }

}
