#include "masterpointswindow.h"
#include "data_base.h"
#include "createraport.h"
#include "util.h"
#include <QApplication>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTabWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QLabel>
#include <QHeaderView>
#include <QPointer>
#include <QLineSeries>
#include <QDateTime>

MasterPointsWindow::MasterPointsWindow(DataBase& data_base, CreateRaport& create_raport, QWidget* parent)
    : QWidget(parent), data_base_(data_base), create_raport_(create_raport){
    setWindowTitle("Таблица для интерполяции");
    QVBoxLayout *vbox_master = new QVBoxLayout(this);
    QTabWidget *tab_master = new QTabWidget(this);
    QPointer<QWidget> widget_acm = CreateTabMasterACM();
    QPointer<QWidget> widget_las = CreateTabMasterLAS();
    tab_master->addTab(widget_acm,"GST");
    tab_master->addTab(widget_las,"LAS");
    QGroupBox* group_source = new QGroupBox("Источник данных");
    QHBoxLayout* hbox_source = new QHBoxLayout();
    hbox_source->setAlignment(Qt::AlignLeft);
    QRadioButton* rad_cp = new QRadioButton("Контрольные точки");
    QRadioButton* rad_ranges = new QRadioButton("Контрольные диапазоны");
    rad_cp->setChecked(true);
    connect(rad_ranges, &QRadioButton::toggled, this, [this](bool on){ use_ranges_ = on; });
    hbox_source->addWidget(rad_cp);
    hbox_source->addWidget(rad_ranges);
    group_source->setLayout(hbox_source);
    vbox_master->addWidget(group_source);
    vbox_master->addWidget(tab_master);
}

QVector<double> MasterPointsWindow::ActiveTemp(){
    if(use_ranges_){
        QVector<double> v;
        for(const CheckRange& r : data_base_.GetCheckRanges()){
            v.push_back(r.avg_temp);
        }
        return v;
    }
    return data_base_.GetCheckPointTemp();
}

QVector<double> MasterPointsWindow::ActiveBar(){
    if(use_ranges_){
        QVector<double> v;
        for(const CheckRange& r : data_base_.GetCheckRanges()){
            v.push_back(r.avg_bar);
        }
        return v;
    }
    return data_base_.GetCheckPointBar();
}

double MasterPointsWindow::AverageSeriesOverRange(QLineSeries* series, const CheckRange& range){
    if(!series){
        return 0;
    }
    qint64 a = range.t_start.toMSecsSinceEpoch();
    qint64 b = range.t_end.toMSecsSinceEpoch();
    double sum = 0;
    int count = 0;
    for(const QPointF& p : series->points()){
        qint64 ms = RoundToSec(static_cast<qint64>(p.x()));
        if(ms >= a && ms <= b){
            sum += p.y();
            count++;
        }
    }
    return count ? sum / count : 0;
}

void MasterPointsWindow::FillGridFromRanges(QLineSeries* series, QStandardItemModel* model){
    if(!series || !model){
        return;
    }
    int etalon_bar = s_et_bar_->value();
    int row = 1;
    int column = 1;
    for(const CheckRange& r : data_base_.GetCheckRanges()){
        QStandardItem *it = new QStandardItem(QString::number(AverageSeriesOverRange(series, r)));
        it->setTextAlignment(Qt::AlignCenter);
        model->setItem(row, column, it);
        row++;
        if(row > etalon_bar){
            row = 1;
            column++;
        }
    }
}

void MasterPointsWindow::Open(){
    CreateInContentLAS();
    CreateInContentACM();
    show();
}

QWidget* MasterPointsWindow::CreateTabMasterLAS(){
    QWidget *widget_las = new QWidget();
    QVBoxLayout *main_vbox = new QVBoxLayout();
    QHBoxLayout *botton_hbox = new QHBoxLayout();
    botton_hbox->setAlignment(Qt::AlignLeft);
    tab_las_ = new QTabWidget();
    QPushButton *btn_saveas = new QPushButton("Сохранить все");
    connect(btn_saveas,&QPushButton::clicked, this,&MasterPointsWindow::CreateAllDocLAS);
    QPushButton *btn_table = new QPushButton("Заполнить данные");
    connect(btn_table, &QPushButton::clicked,this, [&](){
        FillAllTablesLAS();
    });
    QCheckBox *change_box = new QCheckBox("Редактирование таблицы");
    change_box->setCheckState(Qt::Unchecked);
    botton_hbox->addWidget(btn_saveas);
    s_et_bar_las_ = new QSpinBox();
    s_et_bar_las_->setRange(0,50);
    s_et_bar_las_->setSingleStep(1);
    s_et_bar_las_->setValue(0);
    s_et_temp_las_ = new QSpinBox();
    s_et_temp_las_->setRange(0,50);
    s_et_temp_las_->setSingleStep(1);
    s_et_temp_las_->setValue(0);
    s_step_bar_las_ = new QDoubleSpinBox();
    s_step_bar_las_->setRange(0,2000);
    s_step_bar_las_->setSingleStep(1);
    s_step_bar_las_->setValue(0);
    s_step_bar_las_->setEnabled(false);
    QVBoxLayout* vbox_group = new QVBoxLayout();
    QHBoxLayout* hbox_1 = new QHBoxLayout();
    QHBoxLayout* hbox_2 = new QHBoxLayout();
    QGroupBox* group_select_canal = new QGroupBox("Настройка каналов");
    ch_sel_1_table_las_ = new QCheckBox("Наличие термокомпенсации");
    ch_sel_1_table_las_->setCheckState(Qt::Unchecked);
    QVBoxLayout* vbox_sel_canal = new QVBoxLayout();
    QHBoxLayout* hbox_sel_canal_1 = new QHBoxLayout();
    QHBoxLayout* hbox_sel_canal_2 = new QHBoxLayout();
    QComboBox* combo_name_1 = new QComboBox;
    combo_name_1->addItems(chanels_);
    combo_name_1->setCurrentIndex(4);
    name_canal_las_1_ = "Р-Давление";
    name_canal_las_2_ = "Е-Термокомпенсация";
    connect(combo_name_1, &QComboBox::currentTextChanged, this, [&](const QString &text){
        name_canal_las_1_ = text;
        CreateInContentLAS();
    });
    QComboBox* combo_name_2 = new QComboBox;
    combo_name_2->addItems(chanels_);
    combo_name_2->setCurrentIndex(3);
    connect(combo_name_2, &QComboBox::currentTextChanged, this, [&](const QString &text){
        name_canal_las_2_ = text;
        CreateInContentLAS();
    });
    connect(ch_sel_1_table_las_, &QCheckBox::toggled, this,[this](){
        flag_termocompens_ = ch_sel_1_table_las_->isChecked();
    });
    hbox_sel_canal_1->setAlignment(Qt::AlignLeft);
    hbox_sel_canal_2->setAlignment(Qt::AlignLeft);
    hbox_sel_canal_1->addWidget(combo_name_1);
    hbox_sel_canal_2->addWidget(combo_name_2);
    vbox_sel_canal->addWidget(ch_sel_1_table_las_);
    vbox_sel_canal->addLayout(hbox_sel_canal_1);
    vbox_sel_canal->addLayout(hbox_sel_canal_2);
    group_select_canal->setLayout(vbox_sel_canal);
    QGroupBox *group_etalon_temp = new QGroupBox("Эталонные значения");
    QLabel *l_et_bar = new QLabel("Кол.точек температур");
    QLabel *l_et_temp = new QLabel("Кол.точек давлений");
    QLabel *l_step_bar = new QLabel("Шаг давления атм. ");
    check_step_bar_las_ = new QCheckBox();
    connect(check_step_bar_las_, &QCheckBox::clicked, this,[=](){
        s_step_bar_las_->setEnabled(check_step_bar_las_->isChecked());
    });
    connect(change_box, &QCheckBox::clicked, this,[=](){
        if(change_box->isChecked()){
            change_all_tables_ = true;
            btn_table->setEnabled(false);
        } else {
            change_all_tables_ = false;
            btn_table->setEnabled(true);
        }
    });
    hbox_1->addWidget(l_et_bar);
    hbox_1->addWidget(s_et_temp_las_);
    hbox_1->setAlignment(Qt::AlignLeft);
    hbox_2->addWidget(l_et_temp);
    hbox_2->addWidget(s_et_bar_las_);
    hbox_2->setAlignment(Qt::AlignLeft);
    hbox_2->addWidget(check_step_bar_las_);
    hbox_2->addWidget(l_step_bar);
    hbox_2->addWidget(s_step_bar_las_);
    hbox_2->setAlignment(Qt::AlignLeft);
    vbox_group->addLayout(hbox_1);
    vbox_group->addLayout(hbox_2);
    vbox_group->addWidget(btn_table,0,Qt::AlignLeft);
    vbox_group->addWidget(change_box,0,Qt::AlignLeft);
    group_etalon_temp->setLayout(vbox_group);
    main_vbox->addWidget(group_select_canal);
    main_vbox->addWidget(group_etalon_temp);
    main_vbox->addWidget(tab_las_);
    main_vbox->addLayout(botton_hbox);
    widget_las->setLayout(main_vbox);
    return widget_las;
}

void MasterPointsWindow::CreateInContentLAS(){
    ClearTabWidget(tab_las_);
    for(auto& data : data_base_.GetDataSerACM()){
        QVector<Canal>& acm = data.vec_canal;
        QWidget *tab_sensor = new QWidget();
        QVBoxLayout *vbox = new QVBoxLayout();
        QGroupBox *group_bar = new QGroupBox("Таблица");
        QVBoxLayout *vbox_bar = new QVBoxLayout();
        QHBoxLayout *table_hbox = new QHBoxLayout();
        QTableView *table_bar = new QTableView();
        QPointer<QStandardItemModel>model_bar;
        for(Canal& a :acm){
            if(a.name_canal.contains(name_canal_las_1_,Qt::CaseInsensitive)){
                 model_bar = a.model;
            }
            connect(a.model, &QStandardItemModel::itemChanged,this,[&](QStandardItem *item){
                if(change_all_tables_){
                    ChangeAllTables(item);
                }
            });
        }
        QHBoxLayout *btn_hbox = new QHBoxLayout();
        QPushButton *btn_create_doc = new QPushButton("Создать отчет");
        QPushButton *btn_delete_row = new QPushButton("Удалить строку");
        connect(btn_delete_row, &QPushButton::clicked, this,[=](){
            if(table_bar->selectionModel()->hasSelection()){
                QModelIndex index_bar = table_bar->currentIndex();
                if(index_bar.isValid()){
                    model_bar->removeRow(index_bar.row());
                    table_bar->clearSelection();
                }
            }
        });
        QPushButton *btn_delete_col = new QPushButton("Удалить столбец");
        connect(btn_delete_col, &QPushButton::clicked, this,[=](){
            if(table_bar->selectionModel()->hasSelection()){
                QModelIndex index_bar = table_bar->currentIndex();
                if(index_bar.isValid()){
                    model_bar->removeColumn(index_bar.column());
                    table_bar->clearSelection();
                }
            }
        });
        const QString sensor_name = data.name_sensor;   // копия по значению
        connect(btn_create_doc, &QPushButton::clicked, this, [this, sensor_name](){
            DataSeriesSensor* data = FindSensorData(sensor_name);
            if (!data) return;
            QString path = QApplication::applicationDirPath();
            QString path_doc = QFileDialog::getSaveFileName(nullptr, "Сохранить отчет", path ,"*.txt");
            if (path_doc.isEmpty()) return;
            if (data_base_.GetCheckPoints().isEmpty()) return;
            QString date =  data_base_.GetCheckPoints()[0].toString("dd.MM.yyyy");
            QString file_data ="/" + data->name_sensor.trimmed() + "_PT_" + date + ".txt";
            QString file_name = path_doc +  file_data ;
            create_raport_.CreateDocLAS(*data,file_name,name_canal_1_,date);
        });
        QPushButton *btn_append_row = new QPushButton("Добавить строку");
        connect(btn_append_row,&QPushButton::clicked, this,[=](){
            QList<QStandardItem*> row;
            for(int i = 0; i < model_bar->columnCount();++i){
                row << new QStandardItem("");
            }
            model_bar->appendRow(row);
        });
        QPushButton *btn_append_col = new QPushButton("Добавить столбец");
        connect(btn_append_col,&QPushButton::clicked, this,[=](){
            QList<QStandardItem*> col;
            for(int i = 0; i < model_bar->rowCount();++i){
                col<< new QStandardItem("");
            }
            model_bar->appendColumn(col);
        });
        btn_hbox->addWidget(btn_create_doc);
        btn_hbox->addWidget(btn_delete_col);
        btn_hbox->addWidget(btn_delete_row);
        btn_hbox->addWidget(btn_append_row);
        btn_hbox->addWidget(btn_append_col);
        btn_hbox->setAlignment(Qt::AlignLeft);
        table_bar->setModel(model_bar);
        table_bar->horizontalHeader()->hide();
        table_bar->verticalHeader()->hide();
        vbox_bar->addWidget(table_bar);
        group_bar->setLayout(vbox_bar);
        table_hbox->addWidget(group_bar);
        vbox->addLayout(table_hbox);
        vbox->addLayout(btn_hbox);
        tab_sensor->setLayout(vbox);
        tab_las_->addTab(tab_sensor,data.name_sensor);
    }
}

void MasterPointsWindow::FilingTableLAS(QStandardItemModel* model){
    QVector<double> vec_temp = ActiveTemp();
    QVector<double> vec_bar = ActiveBar();
    if(model){
        model->setColumnCount(s_et_temp_las_->value()+1);
        model->setRowCount(s_et_bar_las_->value()+1);
        model->setItem(0,0,new QStandardItem("Трепер"));
    }
    double step_bar = s_step_bar_las_->value();
    double bar_now = 0;
    for(int i = 2 ; i <= s_et_bar_las_->value()+1;++i){
        if(model){
            QStandardItem *it = new QStandardItem(QString::number(bar_now));
            it->setTextAlignment(Qt::AlignCenter);
            model->setItem(0,i,it);
        }
        if(check_step_bar_las_->isChecked()){
            bar_now += step_bar;
        } else {
            bar_now = vec_bar[i-1];
        }
    }
    int index_temp = 0;
    int step_temp = 0;
    if(flag_termocompens_){
        step_temp = 3;
    } else {
        step_temp = 2;
    }
    for( int y = 1 ; y <= s_et_temp_las_->value()* step_temp;y += step_temp){
        QStandardItem *it_b = new QStandardItem(QString::number(vec_temp[index_temp]));
        it_b->setTextAlignment(Qt::AlignCenter);
        QStandardItem *it = new QStandardItem(QString::number(vec_temp[index_temp]));
        it->setTextAlignment(Qt::AlignCenter);
        if(model){
            model->setItem(y,0,it);
            model->setItem(y,1,new QStandardItem("P этал."));
            model->setItem(y+1,0,new QStandardItem());
            model->setItem(y+1,1,new QStandardItem("Код Р"));
            if(flag_termocompens_){
                model->setItem(y+2,0,new QStandardItem());
                model->setItem(y+2,1,new QStandardItem("Код k_E"));
            }
        }
        index_temp += s_et_bar_las_->value();
    }
}

void MasterPointsWindow::FillAllTablesLAS(){
    if(!data_base_.GetDataSerACM().isEmpty()){
        for(DataSeriesSensor& data : data_base_.GetDataSerACM()){
            QVector<Canal>& acm = data.vec_canal;
            QPointer<QStandardItemModel> model;
            for(Canal& a :acm){
                if(a.name_canal.contains(name_canal_las_1_,Qt::CaseInsensitive)){
                    model = a.model;
                }
            }
            FilingTableLAS(model);
        }
        for(DataSeriesSensor& data : data_base_.GetDataSerACM()){
            AnalisingSeriesLAS(data);
        }
    }
}

void MasterPointsWindow::AnalisingSeriesLAS(const DataSeriesSensor& data){
    const QVector<Canal>& acm = data.vec_canal;
    QPointer<QLineSeries> series;
    QPointer<QStandardItemModel> model;
    QPointer<QLineSeries> series_term;
    const QVector<QDateTime>& vec = data_base_.GetCheckPoints();
    const int n = vec.size();
    const QVector<double>& vec_bar_etalon = data_base_.GetCheckPointBar();
    for(const Canal& a :acm){
        if(a.name_canal.contains(name_canal_las_1_,Qt::CaseInsensitive)){
            series = a.series;
            model = a.model;
        }
        if(a.name_canal.contains(name_canal_las_2_,Qt::CaseInsensitive) && flag_termocompens_){
            series_term = a.series;
        }
    }
    if(use_ranges_){
        QVector<CheckRange>& ranges = data_base_.GetCheckRanges();
        int etalon_bar_r = s_et_bar_las_->value();
        int step_row_r = flag_termocompens_ ? 3 : 2;
        int row_r = 2;
        int column_r = 2;
        for(const CheckRange& r : ranges){
            if(model){
                QStandardItem *it_etlon = new QStandardItem(QString::number(r.avg_bar));
                it_etlon->setTextAlignment(Qt::AlignCenter);
                model->setItem(row_r-1, column_r, it_etlon);
                QStandardItem *it = new QStandardItem(QString::number(AverageSeriesOverRange(series, r)));
                it->setTextAlignment(Qt::AlignCenter);
                model->setItem(row_r, column_r, it);
            }
            column_r++;
            if(column_r > etalon_bar_r+1){
                row_r += step_row_r;
                column_r = 2;
            }
        }
        if(series_term){
            row_r = 3;
            column_r = 2;
            for(const CheckRange& r : ranges){
                if(model){
                    QStandardItem *it = new QStandardItem(QString::number(AverageSeriesOverRange(series_term, r)));
                    it->setTextAlignment(Qt::AlignCenter);
                    model->setItem(row_r, column_r, it);
                }
                column_r++;
                if(column_r > etalon_bar_r+1){
                    row_r += step_row_r;
                    column_r = 2;
                }
            }
        }
        return;
    }
    int count = 0;
    int etalon_bar = s_et_bar_las_->value();
    int step_row;
    if(flag_termocompens_){
        step_row = 3;
    } else{
        step_row = 2;
    }
    int row = 2;
    int column = 2;
    if(series){
        for(QPointF& point : series->points()){
            if(count == n){
                break;
            }
            qint64 t = static_cast<qint64>(point.x());
            qint64 res = RoundToSec(t);
            qint64 res_2 = RoundToSec(vec[count].toMSecsSinceEpoch());
            QDateTime check_time = QDateTime::fromMSecsSinceEpoch(res_2);
            QDateTime time = QDateTime::fromMSecsSinceEpoch(res);
            if(time == check_time){
                QStandardItem *it_etlon = new QStandardItem(QString::number(vec_bar_etalon[count]));
                it_etlon->setTextAlignment(Qt::AlignCenter);
                count++;
                QStandardItem *it = new QStandardItem(QString::number(point.y()));
                it->setTextAlignment(Qt::AlignCenter);
                if(model){
                    model->setItem(row-1,column,it_etlon);
                    model->setItem(row,column,it);
                }
                column++;
            }
            if(column > etalon_bar+1){
                row += step_row;
                column = 2;
            }
        }
        count = 0;
        row = 3;
        column = 2;
    }
    if(series_term){
        for(QPointF& point : series_term->points()){
            if(count == n){
                break;
            }
            qint64 t = static_cast<qint64>(point.x());
            qint64 res = RoundToSec(t);
            qint64 res_2 = RoundToSec(vec[count].toMSecsSinceEpoch());
            QDateTime check_time = QDateTime::fromMSecsSinceEpoch(res_2);
            QDateTime time = QDateTime::fromMSecsSinceEpoch(res);
            if(time == check_time){
                count++;
                QStandardItem *it = new QStandardItem(QString::number(point.y()));
                it->setTextAlignment(Qt::AlignCenter);
                if(model){
                    model->setItem(row,column,it);
                }
                column++;
            }
            if(column > etalon_bar+1){
                row += step_row;
                column = 2;
            }
        }
    }

}

QWidget* MasterPointsWindow::CreateTabMasterACM(){
    QWidget *widget_acm = new QWidget();
    QVBoxLayout *main_vbox = new QVBoxLayout();
    QHBoxLayout *botton_hbox = new QHBoxLayout();
    botton_hbox->setAlignment(Qt::AlignLeft);
    tab_ = new QTabWidget();
    QPushButton *btn_saveas = new QPushButton("Сохранить все");
    connect(btn_saveas,&QPushButton::clicked, this,&MasterPointsWindow::CreateAllDoc);
    QPushButton *btn_table = new QPushButton("Заполнить данные");
    connect(btn_table, &QPushButton::clicked,this, [&](){
        if(ch_sel_1_table_las_->isChecked()){
            FillFromOneTable();
        } else {
            FillAllTables();
        }
    });
    QCheckBox *change_box = new QCheckBox("Редактирование таблицы");
    change_box->setCheckState(Qt::Unchecked);
    botton_hbox->addWidget(btn_saveas);
    s_et_bar_ = new QSpinBox();
    s_et_bar_->setRange(0,50);
    s_et_bar_->setSingleStep(1);
    s_et_bar_->setValue(0);
    s_et_temp_ = new QSpinBox();
    s_et_temp_->setRange(0,50);
    s_et_temp_->setSingleStep(1);
    s_et_temp_->setValue(0);
    s_step_bar_ = new QDoubleSpinBox();
    s_step_bar_->setRange(0,2000);
    s_step_bar_->setSingleStep(1);
    s_step_bar_->setValue(0);
    s_step_bar_->setEnabled(false);
    QVBoxLayout* vbox_group = new QVBoxLayout();
    QHBoxLayout* hbox_1 = new QHBoxLayout();
    QHBoxLayout* hbox_2 = new QHBoxLayout();
    QGroupBox* group_select_canal = new QGroupBox("Выбор каналов для таблиц");
    ch_sel_1_table_ = new QCheckBox("Работа с одной таблицей");
    ch_sel_1_table_->setCheckState(Qt::Unchecked);
    QVBoxLayout* vbox_sel_canal = new QVBoxLayout();
    QHBoxLayout* hbox_sel_canal_1 = new QHBoxLayout();
    QHBoxLayout* hbox_sel_canal_2 = new QHBoxLayout();
    QComboBox* combo_name_1 = new QComboBox;
    combo_name_1->addItems(chanels_);
    combo_name_1->setCurrentIndex(2);
    name_canal_1_ = "Т-Температура";
    name_canal_2_ = "Р-Давление";
    connect(combo_name_1, &QComboBox::currentTextChanged, this, [&](const QString &text){
        name_canal_1_ = text;
        CreateInContentACM();
    });
    QComboBox* combo_name_2 = new QComboBox;
    combo_name_2->addItems(chanels_);
    combo_name_2->setCurrentIndex(4);
    connect(combo_name_2, &QComboBox::currentTextChanged, this, [&](const QString &text){
        name_canal_2_ = text;
        CreateInContentACM();
    });
    connect(ch_sel_1_table_, &QCheckBox::toggled, this,[this,combo_name_2](){
        if(ch_sel_1_table_->isChecked()){
            s_step_bar_->setEnabled(false);
            s_et_bar_->setEnabled(false);
            combo_name_2->setEnabled(false);
            check_step_bar_->setEnabled(false);
        }else {
            s_step_bar_->setEnabled(true);
            s_et_bar_->setEnabled(true);
            combo_name_2->setEnabled(true);
            check_step_bar_->setEnabled(true);
        }
    });
    QLabel* l_table_1 = new QLabel("Таблица 1");
    QLabel* l_table_2 = new QLabel("Таблица 2");
    hbox_sel_canal_1->setAlignment(Qt::AlignLeft);
    hbox_sel_canal_2->setAlignment(Qt::AlignLeft);
    hbox_sel_canal_1->addWidget(l_table_1);
    hbox_sel_canal_1->addWidget(combo_name_1);
    hbox_sel_canal_2->addWidget(l_table_2);
    hbox_sel_canal_2->addWidget(combo_name_2);
    vbox_sel_canal->addWidget(ch_sel_1_table_);
    vbox_sel_canal->addLayout(hbox_sel_canal_1);
    vbox_sel_canal->addLayout(hbox_sel_canal_2);
    group_select_canal->setLayout(vbox_sel_canal);
    QGroupBox *group_etalon_temp = new QGroupBox("Эталонные значения");
    QLabel *l_et_bar = new QLabel("Кол.точек температур");
    QLabel *l_et_temp = new QLabel("Кол.точек давлений");
    QLabel *l_step_bar = new QLabel("Шаг давления атм. ");
    check_step_bar_ = new QCheckBox();
    connect(check_step_bar_, &QCheckBox::clicked, this,[=](){
        s_step_bar_->setEnabled(check_step_bar_->isChecked());
    });
    connect(change_box, &QCheckBox::clicked, this,[=](){
        if(change_box->isChecked()){
            change_all_tables_ = true;
            btn_table->setEnabled(false);
        } else {
            change_all_tables_ = false;
            btn_table->setEnabled(true);
        }
    });
    hbox_1->addWidget(l_et_bar);
    hbox_1->addWidget(s_et_temp_);
    hbox_1->setAlignment(Qt::AlignLeft);
    hbox_2->addWidget(l_et_temp);
    hbox_2->addWidget(s_et_bar_);
    hbox_2->setAlignment(Qt::AlignLeft);
    hbox_2->addWidget(check_step_bar_);
    hbox_2->addWidget(l_step_bar);
    hbox_2->addWidget(s_step_bar_);
    hbox_2->setAlignment(Qt::AlignLeft);
    vbox_group->addLayout(hbox_1);
    vbox_group->addLayout(hbox_2);
    vbox_group->addWidget(btn_table,0,Qt::AlignLeft);
    vbox_group->addWidget(change_box,0,Qt::AlignLeft);
    group_etalon_temp->setLayout(vbox_group);
    main_vbox->addWidget(group_select_canal);
    main_vbox->addWidget(group_etalon_temp);
    main_vbox->addWidget(tab_);
    main_vbox->addLayout(botton_hbox);
    widget_acm->setLayout(main_vbox);
    return widget_acm;
}

void MasterPointsWindow::CreateInContentACM(){
    ClearTabWidget(tab_);
    for(auto& data : data_base_.GetDataSerACM()){
        QVector<Canal>& acm = data.vec_canal;
        QWidget *tab_sensor = new QWidget();
        QVBoxLayout *vbox = new QVBoxLayout();
        QGroupBox *group_temp = new QGroupBox("Таблица 2");
        QVBoxLayout *vbox_temp = new QVBoxLayout();
        QGroupBox *group_bar = new QGroupBox("Таблица 1");
        QVBoxLayout *vbox_bar = new QVBoxLayout();
        QHBoxLayout *table_hbox = new QHBoxLayout();
        QTableView *table_bar = new QTableView();
        QTableView *table_temp = new QTableView();
        QPointer<QStandardItemModel> model_bar;
        QPointer<QStandardItemModel> model_temp;
        for(Canal& a :acm){
            if(a.name_canal.contains(name_canal_1_,Qt::CaseInsensitive)){
                model_bar = a.model;
            }
            if(a.name_canal.contains(name_canal_2_,Qt::CaseInsensitive)){
                model_temp = a.model;
            }
            disconnect(a.model, &QStandardItemModel::itemChanged, this, nullptr);
            connect(a.model, &QStandardItemModel::itemChanged,this,[&](QStandardItem *item){
                if(change_all_tables_){
                    ChangeAllTables(item);
                }
            });
        }
        QHBoxLayout *btn_hbox = new QHBoxLayout();
        QPushButton *btn_create_doc = new QPushButton("Создать отчет");
        QPushButton *btn_delete_row = new QPushButton("Удалить строку");
        connect(btn_delete_row, &QPushButton::clicked, this,[=](){
            if(table_bar->selectionModel()->hasSelection()){
                QModelIndex index_bar = table_bar->currentIndex();
                if(index_bar.isValid()){
                    model_bar->removeRow(index_bar.row());
                    table_bar->clearSelection();
                }
            }
            if(table_temp->selectionModel()->hasSelection()){
                QModelIndex index_temp = table_temp->currentIndex();
                if(index_temp.isValid()){
                    model_temp->removeRow(index_temp.row());
                    table_temp->clearSelection();
                }
            }
        });
        QPushButton *btn_delete_col = new QPushButton("Удалить столбец");
        connect(btn_delete_col, &QPushButton::clicked, this,[=](){
            if(table_bar->selectionModel()->hasSelection()){
                QModelIndex index_bar = table_bar->currentIndex();
                if(index_bar.isValid()){
                    model_bar->removeColumn(index_bar.column());
                    table_bar->clearSelection();
                }
            }
            if(table_temp->selectionModel()->hasSelection()){
                QModelIndex index_temp = table_temp->currentIndex();
                if(index_temp.isValid()){
                    model_temp->removeColumn(index_temp.column());
                    table_temp->clearSelection();
                }
            }
        });
        const QString sensor_name = data.name_sensor;
        connect(btn_create_doc, &QPushButton::clicked, this, [this, sensor_name](){
            DataSeriesSensor* data = FindSensorData(sensor_name);
            if (!data) return;
            QString path = QApplication::applicationDirPath();
            QString path_doc = QFileDialog::getSaveFileName(nullptr, "Сохранить отчет", path, "*.txt");
            if (path_doc.isEmpty()) return;
            if (ch_sel_1_table_->isChecked())
                create_raport_.CreateShortDoc(*data, path_doc, name_canal_1_);
            else
                create_raport_.CreateDoc(*data, path_doc);
        });
        QPushButton *btn_append_row = new QPushButton("Добавить строку");
        connect(btn_append_row,&QPushButton::clicked, this,[=](){
            QList<QStandardItem*> row;
            QList<QStandardItem*> row2;
            for(int i = 0; i < model_bar->columnCount();++i){
                row << new QStandardItem("");
                row2 << new QStandardItem("");
            }
            model_bar->appendRow(row);
            model_temp->appendRow(row2);
        });
        QPushButton *btn_append_col = new QPushButton("Добавить столбец");
        connect(btn_append_col,&QPushButton::clicked, this,[=](){
            QList<QStandardItem*> col;
            QList<QStandardItem*> col2;
            for(int i = 0; i < model_bar->rowCount();++i){
                col<< new QStandardItem("");
                col2 << new QStandardItem("");
            }
            model_bar->appendColumn(col);
            model_temp->appendColumn(col2);
        });
        btn_hbox->addWidget(btn_create_doc);
        btn_hbox->addWidget(btn_delete_col);
        btn_hbox->addWidget(btn_delete_row);
        btn_hbox->addWidget(btn_append_row);
        btn_hbox->addWidget(btn_append_col);
        btn_hbox->setAlignment(Qt::AlignLeft);

        table_temp->setModel(model_temp);
        table_bar->setModel(model_bar);
        table_temp->horizontalHeader()->hide();
        table_bar->horizontalHeader()->hide();
        table_temp->verticalHeader()->hide();
        table_bar->verticalHeader()->hide();
        vbox_temp->addWidget(table_temp);
        vbox_bar->addWidget(table_bar);
        group_temp->setLayout(vbox_temp);
        group_bar->setLayout(vbox_bar);
        table_hbox->addWidget(group_bar);
        table_hbox->addWidget(group_temp);

        vbox->addLayout(table_hbox);
        vbox->addLayout(btn_hbox);
        tab_sensor->setLayout(vbox);
        tab_->addTab(tab_sensor,data.name_sensor);
    }
}

void MasterPointsWindow::FilingTable(QStandardItemModel* model_temp,QStandardItemModel* model_bar){
    QVector<double> vec_temp = ActiveTemp();
    QVector<double> vec_bar = ActiveBar();
    if(model_temp){
        model_temp->setColumnCount(s_et_temp_->value()+1);
        model_temp->setRowCount(s_et_bar_->value()+1);
        model_temp->setItem(0,0,new QStandardItem("P/T"));
    }
    if(model_bar){
        model_bar->setColumnCount(s_et_temp_->value()+1);
        model_bar->setRowCount(s_et_bar_->value()+1);
        model_bar->setItem(0,0,new QStandardItem("P/T"));
    }
    double step_bar = s_step_bar_->value();
    double bar_now = 0;
    for(int i = 1 ; i <= s_et_bar_->value();++i){
        if(model_bar){
            QStandardItem *it = new QStandardItem(QString::number(bar_now));
            it->setTextAlignment(Qt::AlignCenter);
            model_bar->setItem(i,0,it);
        }
        if(model_temp){
            QStandardItem *it_b = new QStandardItem(QString::number(bar_now));
            it_b->setTextAlignment(Qt::AlignCenter);
            model_temp->setItem(i,0,it_b);
        }
        if(check_step_bar_->isChecked()){
            bar_now += step_bar;
        } else {
            bar_now = vec_bar[i];
        }
    }
    int index_temp = 0;
    for( int y = 1 ; y <= s_et_temp_->value();++y){
        double temp_i = 0;
        if(vec_temp.size() > index_temp){
            temp_i = vec_temp[index_temp];
        }
        QStandardItem *it_b = new QStandardItem(QString::number(temp_i));
        it_b->setTextAlignment(Qt::AlignCenter);
        QStandardItem *it = new QStandardItem(QString::number(temp_i));
        it->setTextAlignment(Qt::AlignCenter);
        if(model_bar){
        model_bar->setItem(0,y,it);
        }
        if(model_temp){
            model_temp->setItem(0,y,it_b);
        }
        index_temp += s_et_bar_->value();
    }
}

void MasterPointsWindow::FillFromOneTable(){
    if(!data_base_.GetDataSerACM().isEmpty()){
        for(DataSeriesSensor& data : data_base_.GetDataSerACM()){
            QVector<Canal>& acm = data.vec_canal;
            QPointer<QLineSeries> series;
            QPointer<QStandardItemModel> model;
            for(Canal& a :acm){
                if(a.name_canal.contains(name_canal_1_,Qt::CaseInsensitive)){
                    series = a.series;
                    model = a.model;
                }
            }
            if(model){
                model->setColumnCount(4);
                model->setRowCount(s_et_temp_->value());
                model->setItem(0,0,new QStandardItem("№ Точки"));
                model->setItem(0,1,new QStandardItem("Время"));
                model->setItem(0,2,new QStandardItem("Значение"));
                model->setItem(0,3,new QStandardItem("Эталон"));
            }
            if(use_ranges_){
                QVector<CheckRange>& ranges = data_base_.GetCheckRanges();
                int row = 1;
                for(int i = 0; i < ranges.size(); ++i){
                    if(model){
                        QStandardItem *it = new QStandardItem(QString::number(i));
                        it->setTextAlignment(Qt::AlignCenter);
                        model->setItem(row,0,it);
                        model->setItem(row,1,new QStandardItem(ranges[i].t_mid.time().toString("hh:mm:ss")));
                        model->setItem(row,2,new QStandardItem(QString::number(AverageSeriesOverRange(series, ranges[i]))));
                        model->setItem(row,3,new QStandardItem(QString::number(ranges[i].avg_temp)));
                    }
                    row++;
                    if(row == s_et_temp_->value()){
                        break;
                    }
                }
                continue;
            }
            const QVector<QDateTime>& vec = data_base_.GetCheckPoints();
            const int n = vec.size();
            const QVector<double>& temp = data_base_.GetCheckPointTemp();
            int count = 0;
            int num_point = 0;
            int row = 1;
            if(series){
                for(QPointF& point : series->points()){
                    if(count >= n){
                        break;
                    }
                    qint64 t = static_cast<qint64>(point.x());
                    qint64 res = RoundToSec(t);
                    QDateTime time = QDateTime::fromMSecsSinceEpoch(res);
                    if(time == vec[count]){
                        QStandardItem *it3 = new QStandardItem(QString::number(point.y()));
                        QStandardItem *it2 = new QStandardItem(time.toString("hh:mm:ss"));
                        QStandardItem *it4 = new QStandardItem(QString::number(temp[count]));
                        QStandardItem *it = new QStandardItem(QString::number(num_point));
                        it->setTextAlignment(Qt::AlignCenter);
                        count += s_et_temp_->value();
                        if(model){
                            model->setItem(row,0,it);
                            model->setItem(row,1,it2);
                            model->setItem(row,2,it3);
                            model->setItem(row,3,it4);
                        }
                        row++;
                    }
                    if(row == s_et_temp_->value()){
                        return;
                    }
                    num_point++;
                }
            }
        }
    }
}

void MasterPointsWindow::FillAllTables(){
    if(!data_base_.GetDataSerACM().isEmpty()){
        for(DataSeriesSensor& data : data_base_.GetDataSerACM()){
            QVector<Canal>& acm = data.vec_canal;
            QPointer<QStandardItemModel> model_bar;
            QPointer<QStandardItemModel> model_temp;
            for(Canal& a :acm){
                if(a.name_canal.contains(name_canal_1_,Qt::CaseInsensitive)){
                    model_bar = a.model;
                }
                if(a.name_canal.contains(name_canal_2_,Qt::CaseInsensitive)){
                    model_temp = a.model;
                }
            }
            FilingTable(model_temp,model_bar);
        }
        for(DataSeriesSensor& data : data_base_.GetDataSerACM()){
            AnalisingSeries(data);
        }
    }
}

void MasterPointsWindow::ChangeAllTables(QStandardItem * item){
    int row = item->row();
    int col = item->column();
    if(!data_base_.GetDataSerACM().isEmpty()){
        for(DataSeriesSensor& data : data_base_.GetDataSerACM()){
            QVector<Canal>& acm = data.vec_canal;
            for(Canal& a :acm){
                if(a.model){
                    QStandardItem *it = a.model->item(row,col);
                    it->setText(item->text());
                }
            }
        }
    }
}

void MasterPointsWindow::AnalisingSeries(const DataSeriesSensor& data){
    const QVector<Canal>& acm = data.vec_canal;
    QPointer<QLineSeries> series_bar;
    QPointer<QLineSeries> series_temp;
    QPointer<QStandardItemModel> model_bar ;
    QPointer<QStandardItemModel> model_temp ;
    for(const Canal& a :acm){
        if(a.name_canal.contains(name_canal_1_,Qt::CaseInsensitive)){
            series_bar = a.series;
            model_bar = a.model;
        }
        if(a.name_canal.contains(name_canal_2_,Qt::CaseInsensitive)){
            series_temp = a.series;
            model_temp = a.model;
        }
    }
    if(use_ranges_){
        FillGridFromRanges(series_bar, model_bar);
        FillGridFromRanges(series_temp, model_temp);
        return;
    }
    const QVector<QDateTime>& vec = data_base_.GetCheckPoints();
    const int n = vec.size();
    int count = 0;
    int etalon_bar = s_et_bar_->value();
    int row = 1;
    int column = 1;
    if(series_bar){
        for(QPointF& point : series_bar->points()){
            if(count == n){
                break;
            }
            qint64 t = static_cast<qint64>(point.x());
            qint64 res = RoundToSec(t);
            qint64 res_2 = RoundToSec(vec[count].toMSecsSinceEpoch());
            QDateTime check_time = QDateTime::fromMSecsSinceEpoch(res_2);
            QDateTime time = QDateTime::fromMSecsSinceEpoch(res);
            if(time == check_time){
                count++;
                QStandardItem *it = new QStandardItem(QString::number(point.y()));
                it->setTextAlignment(Qt::AlignCenter);
                if(model_bar){
                    model_bar->setItem(row,column,it);
                }
                row++;
            }
            if(row > etalon_bar){
                row = 1;
                column++;
            }
        }
        count = 0;
        row = 1;
        column = 1;
    }
    if(series_temp){
        for(QPointF& point : series_temp->points()){
            if(count == n){
                break;
            }
            qint64 t = static_cast<qint64>(point.x());
            qint64 res = RoundToSec(t);
            qint64 res_2 = RoundToSec(vec[count].toMSecsSinceEpoch());
            QDateTime check_time = QDateTime::fromMSecsSinceEpoch(res_2);
            QDateTime time = QDateTime::fromMSecsSinceEpoch(res);
            if(time == check_time){
                count++;
                QStandardItem *it = new QStandardItem(QString::number(point.y()));
                it->setTextAlignment(Qt::AlignCenter);
                if(model_temp){
                    model_temp->setItem(row,column,it);
                }
                row++;
            }
            if(row > etalon_bar){
                row = 1;
                column++;
            }
        }
    }
}

void MasterPointsWindow::CreateAllDoc(){
    if(ch_sel_1_table_->isChecked()){
        create_raport_.CreateAllShortDoc(data_base_.GetDataSerACM(),name_canal_1_);
    } else {
        create_raport_.CreateAllDoc(data_base_.GetDataSerACM());
    }
}

void MasterPointsWindow::CreateAllDocLAS(){
    if(!data_base_.GetCheckPoints().empty()){
        QString date =  data_base_.GetCheckPoints()[0].toString("dd.MM.yyyy");
        create_raport_.CreateAllDocLAS(data_base_.GetDataSerACM(),name_canal_las_1_,date);
    }
}

DataSeriesSensor* MasterPointsWindow::FindSensorData(const QString& name){
    for (DataSeriesSensor& d : data_base_.GetDataSerACM())
        if (d.name_sensor == name)
            return &d;
    return nullptr;
}
