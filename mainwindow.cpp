#include "mainwindow.h"
#include <QSpinBox>
#include <QFileInfo>
#include <QRadioButton>
#include <QTimeEdit>


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
    QAction *import = new QAction("Импорт");
    menu_file->addAction(import);
    QMenu *menu_import = new QMenu();
    import->setMenu(menu_import);
    QMenu *menu_sensor = new QMenu();
    QAction *import_etalon = new QAction("Эталон");
    menu_import->addAction(import_etalon);
    connect(import_etalon, &QAction::triggered,this, &MainWindow::LoadDocumentEtalon);
    QAction *sensor = new QAction("Прибор");
    sensor->setMenu(menu_sensor);
    menu_import->addAction(sensor);
    QAction *import_ACM = new QAction("АЦМ");
    connect(import_ACM, &QAction::triggered,this, &MainWindow::LoadDocumentACM);
    QAction *import_AMT = new QAction("АМT");
    connect(import_AMT, &QAction::triggered,this, &MainWindow::LoadDocumentAMT);
    QAction *import_LAS = new QAction("LAS");
    connect(import_LAS, &QAction::triggered,this, &MainWindow::LoadDocumentLAS);
    menu_sensor->addAction(import_LAS);
    menu_sensor->addAction(import_AMT);
    menu_sensor->addAction(import_ACM);
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
    load_doc_2_ = new QAction("+LAS",tool_bar);
    load_doc_2_->setEnabled(false);
    connect(load_doc_2_, &QAction::triggered, this,&MainWindow::LoadDocumentLAS);
    load_doc_3_ = new QAction("+АЦМ",tool_bar);
    load_doc_3_->setEnabled(false);
    connect(load_doc_3_, &QAction::triggered, this,&MainWindow::LoadDocumentACM);
    load_doc_4_ = new QAction("+АМТ",tool_bar);
    load_doc_4_->setEnabled(false);
    connect(load_doc_4_, &QAction::triggered, this,&MainWindow::LoadDocumentAMT);
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
    shift_check_point_->setEnabled(false);
    connect(shift_check_point_, &QAction::triggered,this, &MainWindow::ShiftCheckPoint);
    QAction *load_doc = new QAction("Открыть Эталон",tool_bar);
    connect(load_doc, &QAction::triggered, this,&MainWindow::LoadDocumentEtalon);
    change_canal_ = new QAction("Редактировать каналы",tool_bar);
    change_canal_->setEnabled(false);
    connect(change_canal_, &QAction::triggered, this,&MainWindow::ActoinWinSaC);
    tool_bar->addAction(load_doc);
    tool_bar->addAction(load_doc_2_);
    tool_bar->addAction(load_doc_3_);
    tool_bar->addAction(load_doc_4_);
    tool_bar->addAction(change_canal_);
    tool_bar->addAction(window_axis_);
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
    win_sens_can_ = new QWidget();
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
        path_doc_ = QFileDialog::getOpenFileNames(this, "Открытие файла", path ,"Текстовый документ (*.txt)");
        first_open_doc_ = true;
    } else {
        path_doc_ = QFileDialog::getOpenFileNames(this, "Открытие файла", save_path_ ,"Текстовый документ (*.txt)");
    }
    if(path_doc_.isEmpty()){
        return;
    }
    for(QString path : path_doc_){
        int count_file = path_doc_.size();
        static int count_now = 1;
        if(path.endsWith(".txt", Qt::CaseInsensitive)){
            DataSeriesSensor data = dow_file_.LoadDocACM(path, count_file, count_now);
            data_sensor_.push_back(data);
            QFileInfo file_info(path);
            save_path_ = file_info.absolutePath();
            ++count_now;
        }
    }
    WindowSensorAndCanal(data_sensor_);
}
void MainWindow::LoadDocumentLAS(){
    if(!first_open_doc_){
        QString path = QApplication::applicationDirPath();
        path_doc_ = QFileDialog::getOpenFileNames(this, "Открытие файла", path ,"Las (*.las)");
        first_open_doc_ = true;
    } else {
        path_doc_ = QFileDialog::getOpenFileNames(this, "Открытие файла", save_path_ ,"Las (*.las)");
    }
    if(path_doc_.isEmpty()){
        return;
    }
    for(QString path : path_doc_){
        if(path.endsWith(".las", Qt::CaseInsensitive)){
            DataSeriesSensor data = las_.DowlandLas(path);
            data_sensor_.push_back(data);
            QFileInfo file_info(path);
            save_path_ = file_info.absolutePath();
        }
    }
    WindowSensorAndCanal(data_sensor_);
}
void MainWindow::LoadDocumentAMT(){
    if(!first_open_doc_){
        QString path = QApplication::applicationDirPath();
        path_doc_ = QFileDialog::getOpenFileNames(this, "Открытие файла", path ,"Текстовый документ (*.txt);");
        first_open_doc_ = true;
    } else {
        path_doc_ = QFileDialog::getOpenFileNames(this, "Открытие файла", save_path_ ,"Текстовый документ (*.txt);");
    }
    if(path_doc_.isEmpty()){
        return;
    }

    for(QString path : path_doc_){
        if(path.endsWith(".txt", Qt::CaseInsensitive)){
            int count_file = path_doc_.size();
            static int count_now = 1;
            QFileInfo file_info(path);
            QString name = file_info.baseName();
            DataSeriesSensor data = dow_file_.LoadDocAMT(path, name, count_file, count_now);
            count_now++;
            data_sensor_.push_back(data);
            save_path_ = file_info.absolutePath();
        }
    }
    WindowSensorAndCanal(data_sensor_);
}
void MainWindow::ActoinWinSaC(){
    if(!data_base_.GetDataSerACM().isEmpty()){
        WindowSensorAndCanal(data_base_.GetDataSerACM());
    }
}
void MainWindow::LoadDocumentEtalon(){
    if(!first_open_etalon_){
        QString path = QApplication::applicationDirPath();
        QString path_doc = QFileDialog::getOpenFileName(this, "Открытие файла", path ,"(*.sml2);;(*.sml);;(*.smv)");
        QFileInfo file_info(path_doc);
        save_path_ = file_info.absolutePath();
        first_open_doc_ = true;
        if(path_doc.isEmpty()){
            return;
        }
        if(path_doc.endsWith(".sml2", Qt::CaseInsensitive)){
            dow_file_.LoadDocEtalon_2v(path_doc);
            chart_view_->PanelLegendEtalon();
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
        load_doc_3_->setEnabled(true);
        load_doc_4_->setEnabled(true);
        toogled_legend_->setEnabled(true);
        shift_series_->setEnabled(true);
        data_in_time_->setEnabled(true);
        window_axis_->setEnabled(true);
        action_series_->setEnabled(true);
        delete_sensor_->setEnabled(true);
        shift_check_point_->setEnabled(true);
        //change_canal_->setEnabled(true);
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
                QLabel *label = new QLabel(acm_unit.name_canal +" "+ data.label_sensor->text());
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
        connect(btn_table, &QPushButton::clicked,this, [&](){
            if(ch_sel_1_table_->isChecked()){
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
        connect(combo_name_1, &QComboBox::currentTextChanged, this, [&](const QString &text){
            name_canal_1_ = text;
            CreateInContent();
        });
        QComboBox* combo_name_2 = new QComboBox;
        combo_name_2->addItems(chanels_);
        connect(combo_name_2, &QComboBox::currentTextChanged, this, [&](const QString &text){
            name_canal_2_ = text;
            CreateInContent();
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
        main_vbox->addWidget(group_select_canal);
        main_vbox->addWidget(group_etalon_temp);
        main_vbox->addWidget(tab_);
        main_vbox->addLayout(botton_hbox);
        window_c_p_->setLayout(main_vbox);
        first_open_3_ = true;
    }
    CreateInContent();
    window_c_p_->show();
}
void MainWindow::CreateInContent(){
    tab_->clear();
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
        QStandardItemModel *model_bar = new QStandardItemModel();
        QStandardItemModel *model_temp = new QStandardItemModel();
        for(Canal& a :acm){
            if(a.name_canal.contains(name_canal_1_,Qt::CaseInsensitive)){
                a.model = model_bar;
            }
            if(a.name_canal.contains(name_canal_2_,Qt::CaseInsensitive)){
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
            if(ch_sel_1_table_->isChecked()){
                create_raport_.CreateShortDoc(data,path_doc,name_canal_1_);
            } else {
                create_raport_.CreateDoc(data,path_doc);
            }
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
void MainWindow::FillFromOneTable(){
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
            const QVector<QDateTime> vec = data_base_.GetCheckPoints();
            const QVector<double> temp = data_base_.GetCheckPointTemp();
            int count = 0;
            int num_point = 0;
            int row = 1;
            if(series){
                for(QPointF& point : series->points()){
                    if(count >= data_base_.GetCheckPoints().size() ){
                        break;
                    }
                    qint64 t = static_cast<qint64>(point.x());
                    qint64 res = ((t + 500) / 1000) * 1000;
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
void MainWindow::FillAllTables(){
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
        if(a.name_canal.contains(name_canal_1_,Qt::CaseInsensitive)){
            series_bar = a.series;
            model_bar = a.model;
        }
        if(a.name_canal.contains(name_canal_2_,Qt::CaseInsensitive)){
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
    if(ch_sel_1_table_->isChecked()){
        create_raport_.CreateAllShortDoc(data_base_.GetDataSerACM(),name_canal_1_);
    } else {
        create_raport_.CreateAllDoc(data_base_.GetDataSerACM());
    }
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
        delete a.axis_y_;
    }
    delete data.label_sensor;
    delete data.line;

}
void MainWindow::WindowCheckPoints(){
        if(!first_open_win_check_points_){
            window_check_points_->setWindowTitle("Контролтные точки");
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
            create_new_point->setLayout(vbox_new_point);
            connect(btn_create, &QPushButton::clicked,this, [=](){
                CreateCheckPoint(s_hour->time());
                chart_view_->ReBuildPointSeries();
                CreateTableCheckPoints();
            });
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
            vbox->addWidget(create_new_point);
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

void MainWindow::CreateCheckPoint(QTime time_read){
    double temp = 0.0;
    double bar = 0.0;
    QDateTime date_study;
    QVector<DataSeriesEtalon>& vec = data_base_.GetDataSerEtalon();
    for(DataSeriesEtalon& v : vec){
        if(v.series){
            for(QPointF point : v.series->points()){
                qint64 t = static_cast<qint64>(point.x());
                qint64 res = ((t + 500) / 1000) * 1000;
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
void MainWindow::ReplaceCheckSeries(){
    chart_view_->ReplaceCheckSeries();
}
void MainWindow::WindowView(){
    if(!first_open_win_view_){
        window_view_ = new QWidget();
        window_view_->setWindowTitle("Вид");
        window_view_->setWindowModality(Qt::ApplicationModal);
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
        rad_1->setChecked(true);
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
        data_base_.GetDataSerEtalon()[0].series->replace(data_temp.points_triangle_view);
        DataSeriesEtalon& data_bar = data_base_.GetDataSerEtalon()[1];
        data_base_.GetDataSerEtalon()[1].series->replace(data_bar.points_triangle_view);
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
        data_base_.GetDataSerEtalon()[0].series->replace(data_temp.points_rectangle_view);
        DataSeriesEtalon& data_bar = data_base_.GetDataSerEtalon()[1];
        data_base_.GetDataSerEtalon()[1].series->replace(data_bar.points_rectangle_view);
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
void MainWindow::WindowSensorAndCanal(QVector<DataSeriesSensor>& vec_data){
    if (first_open_win_sens_can_){
        win_sens_can_->setWindowTitle("Приборы и каналы");
        win_sens_can_->resize(900, 350);
        win_sens_can_->setWindowFlags(Qt::Window
                                      | Qt::WindowMinimizeButtonHint
                                      | Qt::WindowMaximizeButtonHint);
        win_sens_can_->setWindowModality(Qt::ApplicationModal);
        sensor_list_ = new QListWidget();
        sensor_list_->setFixedWidth(150);
        canal_list_ = new QListWidget();
        QHBoxLayout *layout = new QHBoxLayout();
        QVBoxLayout *vlayout = new QVBoxLayout();
        QHBoxLayout *layout_btn = new QHBoxLayout();
        layout_btn->setAlignment(Qt::AlignLeft);
        QPushButton *btn = new QPushButton("OK");
        QPushButton *btn_close = new QPushButton("Отмена");
        connect(btn_close, &QPushButton::clicked, this, &MainWindow::Clear);
        connect(btn, &QPushButton::clicked, this, [this,&vec_data](){
            PanelLegendACM(vec_data);});
        layout->addWidget(sensor_list_);
        layout->addWidget(canal_list_);
        vlayout->addLayout(layout);
        layout_btn->addWidget(btn);
        layout_btn->addWidget(btn_close);
        vlayout->addLayout(layout_btn);
        win_sens_can_->setLayout(vlayout);
        first_open_win_sens_can_ = false;
    }

    QMap<QString, QString> color_map;
    color_map["Чёрный"] = "0, 0, 0";
    color_map["Красный"] = "255, 0, 0";
    color_map["Темно-синий"] = "0, 0, 255";
    color_map["Синий"] = "68, 114, 196";
    color_map["Голубой"] = "155, 194, 230";
    color_map["Фиолетовый"] = "112, 48, 160";
    color_map["Зелёный"] = "0, 126, 57";
    color_map["Коричневый"] = "191, 143, 0";

    sensor_list_->clear();
    for(DataSeriesSensor&  data : vec_data){
        sensor_list_->addItem(data.name_sensor +" "+ data.number_sensor);
    }
    QObject::connect(sensor_list_, &QListWidget::itemSelectionChanged, [this, color_map, &vec_data]() {
        canal_list_->clear();
        if (sensor_list_->selectedItems().isEmpty()){
            return;
        }
        QString sensor_name = sensor_list_->selectedItems().first()->text();
        for (DataSeriesSensor& data : vec_data) {
            if (data.name_sensor +" "+ data.number_sensor != sensor_name){
                continue;
            }
            for (Canal& can_ref : data.vec_canal) {
                QListWidgetItem *item = new QListWidgetItem(canal_list_);
                QWidget *w = new QWidget;
                QHBoxLayout *hbox = new QHBoxLayout(w);
                QCheckBox* check = new QCheckBox(can_ref.name_canal);
                QComboBox* combo_color = new QComboBox;
                QComboBox* combo_name = new QComboBox;
                QComboBox* combo_error = new QComboBox;
                QComboBox* combo_accept = new QComboBox;
                QComboBox* combo_unit = new QComboBox;
                QComboBox* combo_duration_min = new QComboBox;
                QComboBox* combo_duration_max = new QComboBox;
                QCheckBox* check_ACP = new QCheckBox("AЦП");
                combo_name->addItems(chanels_);
                combo_duration_max->addItems({"0","250","400","600","1000",""});
                combo_duration_min->addItems({"0","250","400","600","1000",""});
                combo_color->addItems({"Чёрный","Красный","Синий","Голубой","Фиолетовый","Зелёный","Темно-синий","Коричневый"});
                combo_error->addItems({" ","Абсолютная","Относительная","Приведённая"});
                combo_error->setCurrentText(" ");
                combo_accept->addItems({"0.5","1","1.5","0.15",""});
                combo_unit->addItems({"кгс/см2","МПа","°C","К","мкР/ч","м3/ч",
                                      "См/м","мСм/см","%","-","г/см3","кг/м3"});
                item->setSizeHint(QSize(400, 40));
                hbox->setContentsMargins(0,0,0,0);
                check->setCheckState(Qt::Unchecked);
                check_ACP->setCheckState(Qt::Unchecked);
                combo_duration_max->setEditable(true);
                combo_duration_min->setEditable(true);
                combo_name->setEnabled(false);
                check_ACP->setEnabled(false);
                combo_unit->setEnabled(false);
                combo_error->setEnabled(false);
                combo_accept->setEnabled(false);
                combo_duration_min->setEnabled(false);
                combo_duration_max->setEnabled(false);
                combo_color->setEnabled(false);
                connect(combo_duration_max,&QComboBox::currentIndexChanged, w,[can_ptr = &can_ref, combo_duration_max](const int index){
                    if(combo_duration_max->count() - 1 == index){
                        combo_duration_max->lineEdit()->setReadOnly(false);
                        combo_duration_max->lineEdit()->clear();
                    } else {
                        combo_duration_max->lineEdit()->setReadOnly(true);
                        can_ptr->duration_error_max = combo_duration_max->currentText().toDouble();
                    }
                });
                connect(combo_duration_max->lineEdit(), &QLineEdit::editingFinished,  [can_ptr = &can_ref, combo_duration_max](){
                    bool ok = false;
                    double val = combo_duration_max->lineEdit()->text().toDouble(&ok);
                    if(ok){
                        can_ptr->duration_error_max = val;
                    }
                });
                connect(combo_duration_min,&QComboBox::currentIndexChanged, w,[can_ptr = &can_ref, combo_duration_min](const int index){
                    if(combo_duration_min->count() - 1 == index){
                        combo_duration_min->lineEdit()->setReadOnly(false);
                        combo_duration_min->lineEdit()->clear();
                    } else {
                        combo_duration_min->lineEdit()->setReadOnly(true);
                        can_ptr->duration_error_min = combo_duration_min->currentText().toDouble();
                    }
                });
                connect(combo_duration_min->lineEdit(), &QLineEdit::editingFinished,  [can_ptr = &can_ref, combo_duration_min](){
                    bool ok = false;
                    double val = combo_duration_min->lineEdit()->text().toDouble(&ok);
                    if(ok){
                        can_ptr->duration_error_min = val;
                    }
                });
                combo_accept->setEditable(true);
                connect(combo_accept,&QComboBox::currentIndexChanged, w,[can_ptr = &can_ref, combo_accept](const int index){
                    if(combo_accept->count() - 1 == index){
                        combo_accept->lineEdit()->setReadOnly(false);
                        combo_accept->lineEdit()->clear();
                    } else {
                        combo_accept->lineEdit()->setReadOnly(true);
                        can_ptr->accept_max = combo_accept->currentText().toDouble();
                        can_ptr->accept_min = combo_accept->currentText().toDouble() * (-1);
                    }
                });
                connect(combo_accept->lineEdit(), &QLineEdit::editingFinished,  [can_ptr = &can_ref, combo_accept](){
                    bool ok = false;
                    double val = combo_accept->lineEdit()->text().toDouble(&ok);
                    if(ok){
                        can_ptr->accept_max = val;
                        can_ptr->accept_min = val * (-1);
                    }

                });
                connect(combo_error, &QComboBox::currentTextChanged, w, [can_ptr = &can_ref,combo_error, combo_duration_max, combo_duration_min](const QString &text){
                    if(combo_error->currentText() == "Абсолютная"){
                        can_ptr->type_error = 1;
                        combo_duration_max->setEnabled(false);
                        combo_duration_min->setEnabled(false);
                    }
                    if(combo_error->currentText() == "Относительная"){
                        can_ptr->type_error = 2;
                        combo_duration_max->setEnabled(false);
                        combo_duration_min->setEnabled(false);
                    }
                    if(combo_error->currentText() == "Приведённая"){
                        can_ptr->type_error = 3;
                        combo_duration_max->setEnabled(true);
                        combo_duration_min->setEnabled(true);
                    }
                });
                connect(check, &QCheckBox::toggled, w,[can_ptr = &can_ref,check,combo_name,check_ACP,combo_unit,combo_error,combo_accept,combo_duration_min,combo_duration_max,combo_color](){
                    if(check->isChecked()){
                        can_ptr->select_box = true;
                        combo_name->setEnabled(true);
                        check_ACP->setEnabled(true);
                        combo_unit->setEnabled(true);
                        combo_error->setEnabled(true);
                        combo_accept->setEnabled(true);
                        combo_duration_min->setEnabled(true);
                        combo_duration_max->setEnabled(true);
                        combo_color->setEnabled(true);
                    } else {
                        combo_name->setEnabled(false);
                        check_ACP->setEnabled(false);
                        combo_unit->setEnabled(false);
                        combo_error->setEnabled(false);
                        combo_accept->setEnabled(false);
                        combo_duration_min->setEnabled(false);
                        combo_duration_max->setEnabled(false);
                        combo_color->setEnabled(false);
                        can_ptr->select_box = false;
                    }
                });
                connect(check_ACP, &QCheckBox::clicked, w,[can_ptr = &can_ref,check_ACP,combo_unit,combo_error,combo_accept,combo_duration_min,combo_duration_max](){
                    if(check_ACP->isChecked() && can_ptr->select_box){
                        can_ptr->name_canal = "АЦП_" + can_ptr->name_canal;
                        combo_unit->setCurrentText("-");
                        can_ptr->name_unit = "-";
                        combo_unit->setEnabled(false);
                        can_ptr->check_ACP = true;
                        combo_error->setEnabled(false);
                        combo_accept->setEnabled(false);
                        combo_duration_min->setEnabled(false);
                        combo_duration_max->setEnabled(false);
                    } else {
                        can_ptr->name_canal = can_ptr->name_canal.mid(4);
                        combo_unit->setEnabled(true);
                        can_ptr->check_ACP = false;
                        combo_error->setEnabled(true);
                        combo_accept->setEnabled(true);
                        combo_duration_min->setEnabled(true);
                        combo_duration_max->setEnabled(true);
                    }
                });
                connect(combo_color, &QComboBox::currentTextChanged, w, [can_ptr = &can_ref, color_map](const QString &text){
                    can_ptr->color_series_RGB = color_map.value(text);
                    can_ptr->color_series_ = text;
                });
                connect(combo_name, &QComboBox::currentTextChanged, w, [can_ptr = &can_ref](const QString &text){
                    if(!can_ptr->name_canal.contains("АЦП",Qt::CaseInsensitive)){
                        can_ptr->name_canal = text;
                    }
                });
                connect(combo_unit, &QComboBox::currentTextChanged, w, [can_ptr = &can_ref](const QString &text){
                    can_ptr->name_unit = text;
                });
                if(can_ref.select_box){
                    check->setCheckState(Qt::Checked);
                    int ind =  combo_name->findText(can_ref.name_canal);
                    if(ind == -1){
                        QString name = can_ref.name_canal;
                        combo_name->setCurrentText(name.mid(4));
                    } else {
                        combo_name->setCurrentText(can_ref.name_canal);
                    };
                    if(can_ref.check_ACP){
                        check_ACP->setCheckState(Qt::Checked);
                        combo_unit->setCurrentText("-");
                        combo_unit->setEnabled(false);
                        combo_error->setEnabled(false);
                        combo_accept->setEnabled(false);
                        combo_duration_min->setEnabled(false);
                        combo_duration_max->setEnabled(false);
                    } else {
                        check_ACP->setCheckState(Qt::Unchecked);
                        combo_unit->setEnabled(true);
                        combo_error->setEnabled(true);
                        combo_accept->setEnabled(true);
                        combo_duration_min->setEnabled(true);
                        combo_duration_max->setEnabled(true);
                    }
                    if(can_ref.type_error == 1){
                        combo_error->setCurrentText("Абсолютная");
                        combo_duration_min->setEnabled(false);
                        combo_duration_max->setEnabled(false);
                    }
                    if(can_ref.type_error == 2){
                        combo_error->setCurrentText("Относительная");
                        combo_duration_min->setEnabled(false);
                        combo_duration_max->setEnabled(false);
                    }
                    if(can_ref.type_error == 3){
                        combo_error->setCurrentText("Приведённая");
                        combo_duration_min->setEnabled(true);
                        combo_duration_max->setEnabled(true);
                        combo_duration_min->setCurrentText(QString::number(can_ref.duration_error_min));
                        combo_duration_max->setCurrentText(QString::number(can_ref.duration_error_max));
                    }
                    combo_unit->setCurrentText(can_ref.name_unit);
                    combo_accept->setCurrentText(QString::number(can_ref.accept_max));
                    combo_color->setCurrentText(can_ref.color_series_);
                } else {
                    if(can_ref.name_canal.contains("Температура",Qt::CaseInsensitive)){
                        combo_name->setCurrentText("Т-Температура");
                        combo_unit->setCurrentText("°C");
                        combo_color->setCurrentText("Синий");
                    }
                    if(can_ref.name_canal.contains("Давление",Qt::CaseInsensitive)){
                        combo_name->setCurrentText("Р-Давление");
                        combo_unit->setCurrentText("кгс/см2");
                        combo_color->setCurrentText("Красный");
                    }
                    can_ref.select_box = false;
                    can_ref.accept_min = -1.5;
                    can_ref.accept_max = 1.5;

                    combo_color->setCurrentText("Черный");
                }
                hbox->addWidget(check);
                hbox->addWidget(combo_name);
                hbox->addWidget(check_ACP);
                hbox->addWidget(combo_unit);
                hbox->addWidget(combo_error);
                hbox->addWidget(new QLabel("Допуск ±"));
                hbox->addWidget(combo_accept);
                hbox->addWidget(new QLabel("Диапазон"));
                hbox->addWidget(combo_duration_min);
                hbox->addWidget(combo_duration_max);
                hbox->addWidget(combo_color);
                w->setLayout(hbox);
                canal_list_->setItemWidget(item, w);
            }
        }
    });
    win_sens_can_->show();
}

void MainWindow::Clear(){
    data_sensor_.clear();
    win_sens_can_->hide();
}
void MainWindow::PanelLegendACM(QVector<DataSeriesSensor>& vec_data){
    for(auto& data : vec_data){
        chart_view_->PanelLegendACM(data);
        data_base_.AddDataSerACM(data);
    }
    data_sensor_.clear();
    win_sens_can_->hide();
}
