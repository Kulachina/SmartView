#include "mainwindow.h"
#include <QSpinBox>
#include <QFileInfo>
#include <QRadioButton>

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent),
    data_base_(),
    dow_file_(data_base_),
    create_raport_(data_base_),
    error_table_(data_base_,create_raport_,nullptr)
{
    QMenuBar *menu = menuBar();
    QMenu *menu_file = new QMenu();
    QAction *file = new QAction("Файл");
    file->setMenu(menu_file);
    QAction *open = new QAction("Открыть...");
    connect(open, &QAction::triggered,this, &MainWindow::LoadDocumentEtalon);
    menu_file->addAction(open);
    QAction *save = new QAction("Сохранить");
    connect(save, &QAction::triggered,this, &MainWindow::SaveAllSV);
    menu_file->addAction(save);
    QAction *close = new QAction("Выход");
    menu_file->addAction(close);
    QAction *report = new QAction("Отчеты");
    QAction *master_point = new QAction("Мастер точек");
    connect(master_point, &QAction::triggered, this,&MainWindow::WindowMasterPoint);
    QAction *view = new QAction("Вид");
    connect(view, &QAction::triggered, this,&MainWindow::WindowView);
    QAction *check_point = new QAction("Контрольные точки");
    connect(check_point, &QAction::triggered, this,&MainWindow::WindowCheckPoints);
    QAction *error_delta = new QAction("Таблица погрешностей");
    connect(error_delta, &QAction::triggered, this,&MainWindow::WindowTableError);
    menu->addAction(file);
    menu->addAction(report);
    menu->addAction(view);
    menu->addAction(check_point);
    menu->addAction(master_point);
    menu->addAction(error_delta);
    chart_view_ = new ChartView(nullptr, data_base_);
    QToolBar *tool_bar = new QToolBar();
    load_doc_2_ = new QAction("Добавить прибор",tool_bar);
    load_doc_2_->setEnabled(false);
    connect(load_doc_2_, &QAction::triggered, this,&MainWindow::LoadDocumentACM);
    toogled_legend_ = new QAction("скрыть/показать панель легенд",tool_bar);
    toogled_legend_->setEnabled(false);
    connect(toogled_legend_, &QAction::triggered, this,&MainWindow::ToggledLegendPanel);
    shift_series_ = new QAction("Сдвиг кривых",tool_bar);
    shift_series_->setCheckable(true);
    shift_series_->setEnabled(false);
    connect(shift_series_, &QAction::triggered, this,&MainWindow::ShiftSeries);
    data_in_time_ = new QAction("Данные в точке",tool_bar);
    data_in_time_->setCheckable(true);
    data_in_time_->setEnabled(false);
    connect(data_in_time_, &QAction::triggered, this,&MainWindow::ShiftLineinMouse);
    window_axis_ = new QAction("Окно осей",tool_bar);
    window_axis_->setEnabled(false);
    connect(window_axis_, &QAction::triggered, this,&MainWindow::WindowAxis);
    action_series_ = new QAction("Окно кривых",tool_bar);
    action_series_->setEnabled(false);
    connect(action_series_, &QAction::triggered, this,&MainWindow::WindowSeries);
    delete_sensor_ = new QAction("Удалить прибор");
    delete_sensor_->setEnabled(false);
    connect(delete_sensor_,&QAction::triggered, this, &MainWindow::WindowDeleteSensor);
    shift_check_point_ = new QAction("Сдвиг КТ");
    shift_check_point_->setCheckable(true);
    shift_check_point_->setEnabled(false);
    connect(shift_check_point_, &QAction::triggered,this, &MainWindow::ShiftCheckPoint);
    QAction *load_doc = new QAction("Открыть Эталон",tool_bar);
    connect(load_doc, &QAction::triggered, this,&MainWindow::LoadDocumentEtalon);
    tool_bar->addAction(load_doc);
    tool_bar->addAction(load_doc_2_);
    tool_bar->addAction(window_axis_);
    //tool_bar->addAction(action_series_);
    tool_bar->addAction(toogled_legend_);
    tool_bar->addAction(shift_series_);
    tool_bar->addAction(shift_check_point_);
    tool_bar->addAction(data_in_time_);
    tool_bar->addAction(delete_sensor_);
    addToolBar(tool_bar);
    SetWindow();
    dow_file_.SetChartDoc(chart_view_->GetChart(),chart_view_->GetAxisTemp(),chart_view_->GetAxisBar());
    dow_file_.SetAxisTime(chart_view_->GetAxisX());
    window_check_points_ = new QWidget();
}
MainWindow::~MainWindow()
{

}
void MainWindow::SetWindow(){
    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(chart_view_,8);
    hbox->addWidget(chart_view_->GetWidgetLegend(),2);
    QWidget *w = new QWidget();
    w->setLayout(hbox);
    setCentralWidget(w);
}
void MainWindow::LoadDocumentACM(){
    if(!first_open_doc_){
        QString path = QApplication::applicationDirPath();
        path_doc_ = QFileDialog::getOpenFileNames(this, "Открытие файла", path ,"Текстовый документ (*.txt);;Las (*.las)");
        first_open_doc_ = true;
    } else {
        path_doc_ = QFileDialog::getOpenFileNames(this, "Открытие файла", save_path_ ,"Текстовый документ (*.txt);;Las (*.las)");
    }
    if(path_doc_.isEmpty()){
        return;
    }
    for(QString path : path_doc_){
        if(path.endsWith(".txt", Qt::CaseInsensitive)){
            dow_file_.LoadDocACM(path);
            DataSeriesSensor& data = data_base_.GetDataSerACM().back();
            chart_view_->PanelLegendACM(data);
            QFileInfo file_info(path);
            save_path_ = file_info.absolutePath();
        }
        if(path.endsWith(".las", Qt::CaseInsensitive)){
            DataSeriesSensor& data = las_.DowlandLas(path);
            data_base_.AddDataSerACM(data);
            chart_view_->PanelLegendACM(data_base_.GetDataSerACM().back());
            QFileInfo file_info(path);
            save_path_ = file_info.absolutePath();
        }
    }

}
void MainWindow::LoadDocumentEtalon(){
    if(!first_open_etalon_){
        QString path = QApplication::applicationDirPath();
        QString path_doc = QFileDialog::getOpenFileName(this, "Открытие файла", path ,"SmartLog (*.sml);;SmartView (*.smv)");
        if(path_doc.isEmpty()){
            return;
        }
        if(path_doc.endsWith(".sml", Qt::CaseInsensitive)){
            dow_file_.LoadDocEtalon(path_doc);
            chart_view_->PanelLegendEtalon();
        }
        if(path_doc.endsWith(".smv", Qt::CaseInsensitive)){
            dow_file_.LoadSVDoc(path_doc);
            chart_view_->PanelLegendEtalon();
            for(DataSeriesSensor& data : data_base_.GetDataSerACM())
                chart_view_->PanelLegendACM(data);
        }
        chart_view_->ZoomOn();
        load_doc_2_->setEnabled(true);
        toogled_legend_->setEnabled(true);
        shift_series_->setEnabled(true);
        data_in_time_->setEnabled(true);
        window_axis_->setEnabled(true);
        action_series_->setEnabled(true);
        delete_sensor_->setEnabled(true);
        shift_check_point_->setEnabled(true);
        first_open_etalon_ = true;
    } else {
        int reply = QMessageBox::question(this, "Новый Эталон", "Вы уверены что хоите открыть новый Эталон и потеряете текущий прогресс?",QMessageBox::Yes | QMessageBox::No);
        if(reply == QMessageBox::Yes){
            DeleteAllSens();
            QString path = QApplication::applicationDirPath();
            QString path_doc = QFileDialog::getOpenFileName(this, "Открытие файла", path ,"Формат SmartLog (*.sml);;Формат SmartView (*.smv)");
            if(path_doc.isEmpty()){
                return;
            }
            if(path_doc.endsWith(".sml", Qt::CaseInsensitive)){
                dow_file_.LoadDocEtalon(path_doc);
                chart_view_->PanelLegendEtalon();
            }
            if(path_doc.endsWith(".smv", Qt::CaseInsensitive)){
                dow_file_.LoadSVDoc(path_doc);
                chart_view_->PanelLegendEtalon();
                for(DataSeriesSensor& data : data_base_.GetDataSerACM())
                    chart_view_->PanelLegendACM(data);
            }
        }
    }
}
void MainWindow::ToggledLegendPanel(){
    if(chart_view_->GetWidgetLegend()->isVisible()){
        chart_view_->GetWidgetLegend()->hide();
    } else {
        chart_view_->GetWidgetLegend()->show();
    }
}
void MainWindow::ShiftSeries(){
    chart_view_->ToogledFlagShiftSeries();
}
void MainWindow::ShiftCheckPoint(){
    chart_view_->ToogledFlagShiftCheckPoint();

}
void MainWindow::ShiftLineinMouse(){
    chart_view_->ToogledFlagLineInMouse();
}
void MainWindow::WindowTableError(){
    error_table_.FillTable();
    error_table_.show();

}
void MainWindow::WindowAxis(){
    if(!first_open_){
        window_axes_ = new QWidget();
        window_axes_->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
        window_axes_->setWindowTitle("Настройка осей");
        vbox_axes_ = new QVBoxLayout();
        window_axes_->setLayout(vbox_axes_);
        first_open_ = true;
    }
    while(QLayoutItem *item = vbox_axes_->itemAt(0)){
        if(QLayout *l = item->layout()){
            while(QLayoutItem *it = l->itemAt(0)){
                if(QWidget *w = it->widget()){
                    delete w;
                }
            }
        delete l;
        }
    }
        for(auto& axis : data_base_.GetListAxis()){
            QHBoxLayout *hbox = new QHBoxLayout();
            QCheckBox *check = new QCheckBox();
            if(!axis.isNull()){
                if(axis->isVisible()){
                    check->setCheckState(Qt::Checked);
                } else {
                    check->setCheckState(Qt::Unchecked);
                }
                hbox->addWidget(check);
                hbox->addWidget(new QLabel(axis->titleText()));
                vbox_axes_->addLayout(hbox);
                connect(check, &QCheckBox::toggled, this,[axis,check](){
                    if(check->isChecked()){
                        axis->setVisible(true);
                    } else {
                        axis->setVisible(false);
                    }
                });
            }
        }
    window_axes_->show();
}
void MainWindow::WindowSeries(){
    if(!first_open_2_ && !data_base_.GetDataSerACM().isEmpty()){
        window_series_ = new QWidget();
        window_series_->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
        window_series_->setWindowTitle("Настройка кривых");
        first_open_2_ = true;
    }
    if(first_open_2_){
        QVBoxLayout *vbox = new QVBoxLayout();
        if(data_base_.GetDataSerACM().isEmpty()){
            return;
        }
        for(auto& data : data_base_.GetDataSerACM()){
            QVector<Canal>& acm = data.vec_canal;
            if(acm.isEmpty()){
                return;
            }
            for(Canal& acm_unit : acm){
                QHBoxLayout *hbox = new QHBoxLayout();
                QPointer<QCheckBox> check = acm_unit.check_box;
                QLabel *label = new QLabel(acm_unit.name_type +" "+ data.label_sensor->text());
                hbox->addWidget(check);
                hbox->addWidget(label);
                vbox->addLayout(hbox);
            }
        }
        window_series_->setLayout(vbox);
        window_series_->show();
    }
}
void MainWindow::closeEvent(QCloseEvent *event){
    int reply = QMessageBox::question(this, "Выход", "Вы уверены что хотите выйти?",QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::Yes){
        QApplication::closeAllWindows();
        event->accept();
    } else {
        event->ignore();
    }
}
void MainWindow::WindowMasterPoint(){
    if(!first_open_3_){
        window_c_p_ = new QWidget();
        window_c_p_->setWindowTitle("Таблица для интерполяции");
        QVBoxLayout *main_vbox = new QVBoxLayout();
        QHBoxLayout *botton_hbox = new QHBoxLayout();
        botton_hbox->setAlignment(Qt::AlignLeft);
        tab_ = new QTabWidget();
        QPushButton *btn_saveas = new QPushButton("Сохранить все");
        connect(btn_saveas,&QPushButton::clicked, this,&MainWindow::CreateAllDoc);
        QPushButton *btn_table = new QPushButton("Заполнить таблицы");
        connect(btn_table, &QPushButton::clicked,this, &MainWindow::FillAllTables);
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
        QVBoxLayout *vbox_group = new QVBoxLayout();
        QHBoxLayout *hbox_1 = new QHBoxLayout();
        QHBoxLayout *hbox_2 = new QHBoxLayout();
        QGroupBox *group_etalon_temp = new QGroupBox("Эталонные значения");
        QLabel *l_et_bar = new QLabel("Кол.точек температур");
        QLabel *l_et_temp = new QLabel("Кол.точек давлений");
        QLabel *l_step_bar = new QLabel("Шаг давления атм. ");
        check_step_bar_ = new QCheckBox();
        connect(check_step_bar_, &QCheckBox::clicked, this,[=](){
            if(check_step_bar_->isChecked()){
                s_step_bar_->setEnabled(true);
            } else {
                s_step_bar_->setEnabled(false);
            }
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
        main_vbox->addWidget(group_etalon_temp);
        main_vbox->addWidget(tab_);
        main_vbox->addLayout(botton_hbox);
        window_c_p_->setLayout(main_vbox);
        first_open_3_ = true;
    }
    tab_->clear();
    for(auto& data : data_base_.GetDataSerACM()){
        QVector<Canal>& acm = data.vec_canal;
        QWidget *tab_sensor = new QWidget();
        QVBoxLayout *vbox = new QVBoxLayout();
        QGroupBox *group_temp = new QGroupBox("Температура");
        QVBoxLayout *vbox_temp = new QVBoxLayout();
        QGroupBox *group_bar = new QGroupBox("Давление");
        QVBoxLayout *vbox_bar = new QVBoxLayout();
        QHBoxLayout *table_hbox = new QHBoxLayout();
        QTableView *table_bar = new QTableView();
        QTableView *table_temp = new QTableView();
        QStandardItemModel *model_bar = new QStandardItemModel();
        QStandardItemModel *model_temp = new QStandardItemModel();
        for(Canal& a :acm){
            if(a.name_chart.contains("Давление",Qt::CaseInsensitive)){
                a.model = model_bar;
            }
            if(a.name_chart.contains("Температура",Qt::CaseInsensitive)){
                a.model = model_temp;
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
        connect(btn_create_doc, &QPushButton::clicked,this,[this,&data](){
            QString path = QApplication::applicationDirPath();
            QString path_doc = QFileDialog::getSaveFileName(nullptr, "Сохранить контрольные точки", path ,"*.txt");
            create_raport_.CreateDoc(data,path_doc);
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
    window_c_p_->show();
}
void MainWindow::FilingTable(QStandardItemModel* model_temp,QStandardItemModel* model_bar){
    QVector<double> vec_temp = data_base_.GetCheckPointTemp();
    QVector<double> vec_bar = data_base_.GetCheckPointBar();
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
        QStandardItem *it_b = new QStandardItem(QString::number(vec_temp[index_temp]));
        it_b->setTextAlignment(Qt::AlignCenter);
        QStandardItem *it = new QStandardItem(QString::number(vec_temp[index_temp]));
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
void MainWindow::FillAllTables(){
    if(!data_base_.GetDataSerACM().isEmpty()){
        for(DataSeriesSensor& data : data_base_.GetDataSerACM()){
            QVector<Canal>& acm = data.vec_canal;
            QStandardItemModel *model_bar = new QStandardItemModel();
            QStandardItemModel *model_temp = new QStandardItemModel();
            for(Canal& a :acm){
                if(a.name_chart.contains("Давление",Qt::CaseInsensitive)){
                    model_bar = a.model;
                }
                if(a.name_chart.contains("Температура",Qt::CaseInsensitive)){
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
void MainWindow::ChangeAllTables(QStandardItem * item){
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
void MainWindow::AnalisingSeries(DataSeriesSensor data){
    QVector<Canal>& acm = data.vec_canal;
    QPointer<QLineSeries> series_bar;
    QPointer<QLineSeries> series_temp;
    QPointer<QStandardItemModel> model_bar ;
    QPointer<QStandardItemModel> model_temp ;
    for(Canal& a :acm){
        if(a.name_chart.contains("Давление",Qt::CaseInsensitive)){
            series_bar = a.series;
            model_bar = a.model;
        }
        if(a.name_chart.contains("Температура",Qt::CaseInsensitive)){
            series_temp = a.series;
            model_temp = a.model;
        }
    }
    const QVector<QDateTime> vec = data_base_.GetCheckPoints();
    int count = 0;
    int etalon_bar = s_et_bar_->value();
    int row = 1;
    int column = 1;
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
            if(count == data_base_.GetCheckPoints().size()){
                break;
            }
            qint64 t = static_cast<qint64>(point.x());
            qint64 res = ((t + 500) / 1000) * 1000;
            QDateTime time = QDateTime::fromMSecsSinceEpoch(res);
            if(time == vec[count]){
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
void MainWindow::CreateAllDoc(){
    create_raport_.CreateAllDoc(data_base_.GetDataSerACM());
}
void MainWindow::WindowDeleteSensor(){
    if(!first_open_4_){
        window_del_sens_= new QWidget();
        window_del_sens_->setWindowTitle("Удалить прибор");
        vb_del_sens_ = new QVBoxLayout();
        combo_del_sens_ = new QComboBox();
        QHBoxLayout *hbox = new QHBoxLayout();
        hbox->addWidget(new QLabel("Выберите прибор"));
        hbox->addWidget(combo_del_sens_);
        QHBoxLayout *hbox_btn = new QHBoxLayout();
        QPushButton *btn_del = new QPushButton("Удалить");
        connect(btn_del, &QPushButton::clicked, this, &MainWindow::DeleteOneSens);
        QPushButton *btn_cancel = new QPushButton("Отмена");
        connect(btn_cancel, &QPushButton::clicked, window_del_sens_, &QWidget::hide);
        hbox_btn->addWidget(btn_del,0,Qt::AlignLeft);
        hbox_btn->addWidget(btn_cancel,0,Qt::AlignRight);
        vb_del_sens_->addLayout(hbox);
        vb_del_sens_->addLayout(hbox_btn);
        window_del_sens_->setLayout(vb_del_sens_);
        first_open_4_ = true;
    }
    UpdateComboBox();
    window_del_sens_->show();
}
void MainWindow::UpdateComboBox(){
    QStringList sensors;
    for(DataSeriesSensor data : data_base_.GetDataSerACM()){
        sensors << data.name_sensor;
    }
    combo_del_sens_->clear();
    combo_del_sens_->addItems(sensors);
}
void MainWindow::DeleteOneSens(){
    QString sens = combo_del_sens_->currentText();
    QVector<DataSeriesSensor>& data = data_base_.GetDataSerACM();
    for(auto it = data.begin();it != data.end();){
        if(sens == it->name_sensor){
            DeleteSens(*it);
            it = data.erase(it);
            UpdateComboBox();
            return;
        } else {
            ++it;
        }
    }
}
void MainWindow::DeleteAllSens(){
    QVector<DataSeriesSensor>& data = data_base_.GetDataSerACM();
    for(auto it = data.begin();it != data.end();++it){
            DeleteSens(*it);
    }
    data.clear();
    QVector<DataSeriesEtalon>& dat = data_base_.GetDataSerEtalon();
    for(DataSeriesEtalon& d : dat){
        delete d.series;
        delete d.axis_y_;
        delete d.label_point;
        delete d.data_sensor;
        delete d.point_series;
        if(!d.old_series.isEmpty()){
            for(auto s : d.old_series){
                delete s;
            }
            d.old_series.clear();
        }
    }
    create_axis_ = false;
    dat.clear();
    chart_view_->ClearPanelLegend();
    data_base_.ClearAll();
    dow_file_.ClearAll();
}
void MainWindow::DeleteSens(DataSeriesSensor& data){
    QVector<Canal>& acm = data.vec_canal;
    for(Canal& a : acm){
        delete a.check_box;
        delete a.hbox->itemAt(3)->widget();
        delete a.hbox->itemAt(2)->widget();
        delete a.hbox->itemAt(1)->widget();
        delete a.hbox->itemAt(0)->widget();
        delete a.hbox;
        delete a.label;
        delete a.model;
        delete a.series;
    }
    delete data.label_sensor;
    delete data.line;

}
void MainWindow::WindowCheckPoints(){
    if(!data_base_.GetCheckPoints().isEmpty()){
        if(!first_open_win_check_points_){
            fix_table_ = new QTableView();
            fix_model_ = new QStandardItemModel();
            QPushButton *btn_delete = new QPushButton("Удалить КТ");
            QPushButton *btn_rebuild = new QPushButton("Обновить");
            QHBoxLayout *hbox = new QHBoxLayout();
            QVBoxLayout *vbox = new QVBoxLayout();
            fix_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
            fix_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
            connect(btn_delete,&QPushButton::clicked, this,&MainWindow::DeleteCheckPoint);
            connect(btn_rebuild,&QPushButton::clicked, this,&MainWindow::CreateTableCheckPoints);
            fix_table_->setWindowTitle("Таблица контрольных точек");
            window_check_points_ ->setWindowFlags(Qt::WindowStaysOnTopHint);
            fix_table_->setWindowOpacity(0.9);
            fix_table_->setModel(fix_model_);
            fix_table_->setSortingEnabled(true);
            vbox->addWidget(fix_table_);
            hbox->setAlignment(Qt::AlignLeft);
            hbox->addWidget(btn_delete);
            hbox->addWidget(btn_rebuild);
            vbox->addLayout(hbox);
            window_check_points_->setLayout(vbox);
            first_open_win_check_points_ = true;
            window_check_points_->resize(380,200);
        }
        CreateTableCheckPoints();
        window_check_points_->show();
    }
}
void MainWindow::DeleteCheckPoint(){
    if(fix_table_->selectionModel()->hasSelection()){
        QModelIndex index = fix_table_->currentIndex();
        auto it_time = data_base_.GetCheckPoints().begin();
        auto it_bar = data_base_.GetCheckPointBar().begin();
        auto it_temp = data_base_.GetCheckPointTemp().begin();
        data_base_.GetCheckPointTemp().erase(it_temp+index.row());
        data_base_.GetCheckPointBar().erase(it_bar+index.row());
        data_base_.GetCheckPoints().erase(it_time+index.row());
        CreateTableCheckPoints();
        ReplaceCheckSeries();
        chart_view_->ReBuildPointSeries();
    }
}
void MainWindow::CreateTableCheckPoints(){
    fix_model_->clear();
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
    fix_model_->setHeaderData(0,Qt::Horizontal,"Время");
    fix_model_->setHeaderData(1,Qt::Horizontal,"Давление");
    fix_model_->setHeaderData(2,Qt::Horizontal,"Температура");
    chart_view_->GetChart()->update();
}
void MainWindow::ReplaceCheckSeries(){
    chart_view_->ReplaceCheckSeries();
}
void MainWindow::WindowView(){
    if(!first_open_win_view_){
        window_view_ = new QWidget();
        window_view_->setWindowTitle("Вид");
        window_view_->setWindowFlags(Qt::WindowCloseButtonHint);
        QCheckBox *ch_auto = new QCheckBox("Автомаштаб");
        connect(ch_auto, &QCheckBox::toggled, this, &MainWindow::AutoZoom);
        QGroupBox *gr_box = new QGroupBox("Вид отображения кривых");
        QVBoxLayout *gr_vbox = new QVBoxLayout();
        QVBoxLayout *vbox = new QVBoxLayout();
        QRadioButton *rad_1 = new QRadioButton("В виде прямоугольников");
        connect(rad_1,&QRadioButton::clicked,this,&MainWindow::ReplaceRectangle);
        QRadioButton *rad_2 = new QRadioButton("В виде треугольников");
        connect(rad_2,&QAbstractButton::clicked,this,&MainWindow::ReplaceTriangle);
        rad_2->setChecked(true);
        gr_vbox->setAlignment(Qt::AlignLeft);
        gr_vbox->addWidget(rad_1);
        gr_vbox->addWidget(rad_2);
        gr_box->setLayout(gr_vbox);
        vbox->addWidget(ch_auto);
        vbox->addWidget(gr_box);
        window_view_->setLayout(vbox);
        first_open_win_view_ = true;
    }
    window_view_->show();
}
void MainWindow::ReplaceTriangle(){
    if(!data_base_.GetDataSerACM().isEmpty()){
        for(DataSeriesSensor data : data_base_.GetDataSerACM()){
            QVector<Canal>& acm = data.vec_canal;
            for(Canal& a : acm){
                a.series->replace(a.points_triangle);
            }
        }
    }
    if(!data_base_.GetDataSerEtalon().isEmpty()){
        DataSeriesEtalon& data_temp = data_base_.GetDataSerEtalon()[0];
        data_base_.GetDataSerEtalon()[0].series->replace(data_temp.points_triangle_view_temp);
        DataSeriesEtalon& data_bar = data_base_.GetDataSerEtalon()[1];
        data_base_.GetDataSerEtalon()[1].series->replace(data_bar.points_triangle_view_bar);
    }
    chart_view_->GetChart()->update();
}
void MainWindow::ReplaceRectangle(){
    if(!data_base_.GetDataSerACM().isEmpty()){
        for(DataSeriesSensor data : data_base_.GetDataSerACM()){
            QVector<Canal>& acm = data.vec_canal;
            for(Canal& a : acm){
                a.series->replace(a.points_rectangle);
            }
        }
    }
    if(!data_base_.GetDataSerEtalon().isEmpty()){
        DataSeriesEtalon& data_temp = data_base_.GetDataSerEtalon()[0];
        data_base_.GetDataSerEtalon()[0].series->replace(data_temp.points_rectangle_view_temp);
        DataSeriesEtalon& data_bar = data_base_.GetDataSerEtalon()[1];
        data_base_.GetDataSerEtalon()[1].series->replace(data_bar.points_rectangle_view_bar);
    }

}
void MainWindow::AutoZoom(){
    chart_view_->AutoZoom();
}
void MainWindow::SaveAllSV(){
    chart_view_->ZeroZoom();
    QString path = QApplication::applicationDirPath();
    QString path_doc = QFileDialog::getSaveFileName(nullptr, "Сохранить данных", path ,"Формат SmartView (*.smv);;Текстовый документ (*.txt)");
    if(path_doc.isEmpty()){
        return;
    }
    if(path_doc.endsWith(".smv", Qt::CaseInsensitive)){
        dow_file_.SaveSVDoc(path_doc);
    }

}

