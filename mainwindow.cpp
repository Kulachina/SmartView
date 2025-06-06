#include "mainwindow.h"
#include <QSpinBox>

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent),
    data_base_(),
    dow_file_(data_base_)
{
    QMenuBar *menu = menuBar();
    QMenu *menu_file = new QMenu();
    QAction *file = new QAction("Файл");
    file->setMenu(menu_file);
    QAction *open = new QAction("Открыть...");
    menu_file->addAction(open);
    QAction *close = new QAction("Выход");
    menu_file->addAction(close);
    QAction *report = new QAction("Отчеты");
    QAction *master_point = new QAction("Мастер точек");
    connect(master_point, &QAction::triggered, this,&MainWindow::WindowCheckPoint);
    QAction *view = new QAction("Вид");
    menu->addAction(file);
    menu->addAction(report);
    menu->addAction(view);
    menu->addAction(master_point);
    chart_view_ = new ChartView(nullptr, data_base_);
    QToolBar *tool_bar = new QToolBar();

    load_doc_2 = new QAction("Добавить прибор",tool_bar);
    load_doc_2->setEnabled(false);
    connect(load_doc_2, &QAction::triggered, this,&MainWindow::LoadDocumentACM);
    toogled_legend = new QAction("скрыть/показать панель легенд",tool_bar);
    toogled_legend->setEnabled(false);
    connect(toogled_legend, &QAction::triggered, this,&MainWindow::ToggledLegendPanel);
    shift_series = new QAction("Сдвиг кривых",tool_bar);
    shift_series->setEnabled(false);
    connect(shift_series, &QAction::triggered, this,&MainWindow::ShiftSeries);
    data_in_time = new QAction("Данные в точке",tool_bar);
    data_in_time->setEnabled(false);
    connect(data_in_time, &QAction::triggered, this,&MainWindow::ShiftLineinMouse);
    window_axis = new QAction("Окно осей",tool_bar);
    window_axis->setEnabled(false);
    connect(window_axis, &QAction::triggered, this,&MainWindow::WindowAxis);
    window_series = new QAction("Окно кривых",tool_bar);
    window_series->setEnabled(false);
    connect(window_series, &QAction::triggered, this,&MainWindow::WindowSeries);
    QAction *load_doc = new QAction("Открыть Эталон",tool_bar);
    connect(load_doc, &QAction::triggered, this,&MainWindow::LoadDocumentEtalon);
    tool_bar->addAction(load_doc);
    tool_bar->addAction(load_doc_2);
    tool_bar->addAction(window_axis);
    tool_bar->addAction(window_series);
    tool_bar->addAction(toogled_legend);
    tool_bar->addAction(shift_series);
    tool_bar->addAction(data_in_time);
    addToolBar(tool_bar);
    SetWindow();
    dow_file_.SetChartDoc(chart_view_->GetChart(),chart_view_->GetAxisTemp(),chart_view_->GetAxisBar());
    dow_file_.SetAxisTime(chart_view_->GetAxisX());
}
MainWindow::~MainWindow()
{

}
void MainWindow::SetWindow(){
    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(chart_view_,9);
    hbox->addWidget(chart_view_->GetWidgetLegend(),1);
    QWidget *w = new QWidget();
    w->setLayout(hbox);
    setCentralWidget(w);
}
void MainWindow::LoadDocumentACM(){
    QString path = QApplication::applicationDirPath();
    QString path_doc = QFileDialog::getOpenFileName(this, "Открытие файла", path ,"Текстовый документ (*.txt)");
    if(path_doc.isEmpty()){
        return;
    }
    if(path_doc.endsWith(".txt", Qt::CaseInsensitive)){
        dow_file_.LoadDocACM(path_doc);
        chart_view_->PanelLegendACM();
    }
}
void MainWindow::LoadDocumentEtalon(){
    QString path = QApplication::applicationDirPath();
    QString path_doc = QFileDialog::getOpenFileName(this, "Открытие файла", path ,"Формат SmartLog (*.sml)");
    if(path_doc.isEmpty()){
        return;
    }
    if(path_doc.endsWith(".sml", Qt::CaseInsensitive)){
        dow_file_.LoadDocEtalon(path_doc);
        chart_view_->PanelLegendEtalon();
    }
    load_doc_2->setEnabled(true);
    toogled_legend->setEnabled(true);
    shift_series->setEnabled(true);
    data_in_time->setEnabled(true);
    window_axis->setEnabled(true);
    window_series->setEnabled(true);
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
void MainWindow::ShiftLineinMouse(){
    chart_view_->ToogledFlagLineInMouse();
}
void MainWindow::WindowAxis(){
    if(!first_open_){
        window_axes_ = new QWidget();
        window_axes_->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
        window_axes_->setWindowTitle("Настройка осей");
        QVBoxLayout *vbox = new QVBoxLayout();
        for(auto& axis : data_base_.GetListAxis()){
            QHBoxLayout *hbox = new QHBoxLayout();
            QCheckBox *check = new QCheckBox();
            if(axis->isVisible()){
                check->setCheckState(Qt::Checked);
            } else {
                check->setCheckState(Qt::Unchecked);
            }
            hbox->addWidget(check);
            hbox->addWidget(new QLabel(axis->titleText()));
            vbox->addLayout(hbox);
            connect(check, &QCheckBox::toggled, this,[axis,check](){
                if(check->isChecked()){
                    axis->setVisible(true);
                } else {
                    axis->setVisible(false);
                }
            });
        }
        window_axes_->setLayout(vbox);
        first_open_ = true;
    }
    window_axes_->show();
}
void MainWindow::WindowSeries(){
    if(!first_open_2 && !data_base_.GetDataSerACM().isEmpty()){
        window_series_ = new QWidget();
        window_series_->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
        window_series_->setWindowTitle("Настройка кривых");
        QVBoxLayout *vbox = new QVBoxLayout();
        for(auto& data : data_base_.GetDataSerACM()){
            QHBoxLayout *hbox_temp = new QHBoxLayout();
            QHBoxLayout *hbox_bar = new QHBoxLayout();
            QCheckBox *check_temp = new QCheckBox();
            QCheckBox *check_bar = new QCheckBox();
            check_temp->setCheckState(Qt::Checked);
            check_bar->setCheckState(Qt::Checked);
            QLabel *label_temp = new QLabel("Температура " + data.label_sensor->text());
            label_temp->setStyleSheet("color: blue");
            QLabel *label_bar = new QLabel("Давление " + data.label_sensor->text());
            label_bar->setStyleSheet("color: red");
            hbox_temp->addWidget(check_temp);
            hbox_temp->addWidget(label_temp);
            hbox_bar->addWidget(check_bar);
            hbox_bar->addWidget(label_bar);
            vbox->addLayout(hbox_temp);
            vbox->addLayout(hbox_bar);
            connect(check_temp, &QCheckBox::toggled, this,[data,check_temp](){
                if(check_temp->isChecked()){
                    data.series_temp->setVisible(true);
                } else {
                    data.series_temp->setVisible(false);
                }
            });
            connect(check_bar, &QCheckBox::toggled, this,[data,check_bar](){
                if(check_bar->isChecked()){
                    data.series_bar->setVisible(true);
                } else {
                    data.series_bar->setVisible(false);
                }
            });
        }
        window_series_->setLayout(vbox);
        first_open_2 = true;
    }
    if(first_open_2){
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
void MainWindow::WindowCheckPoint(){
    if(!first_open_3){
        window_c_p = new QWidget();
        window_c_p->setWindowTitle("Таблица для интерполяции");
        QVBoxLayout *main_vbox = new QVBoxLayout();
        QHBoxLayout *botton_hbox = new QHBoxLayout();
        botton_hbox->setAlignment(Qt::AlignLeft);
        tab_ = new QTabWidget();
        QPushButton *btn_saveas = new QPushButton("Сохранить все");
        connect(btn_saveas,&QPushButton::clicked, this,&MainWindow::CreateAllDoc);
        QPushButton *btn_table = new QPushButton("Заполнить таблицы");
        connect(btn_table, &QPushButton::clicked,this, &MainWindow::FillAllTables);
        botton_hbox->addWidget(btn_table);
        botton_hbox->addWidget(btn_saveas);
        s_et_bar_ = new QSpinBox();
        s_et_bar_->setRange(0,50);
        s_et_bar_->setSingleStep(1);
        s_et_bar_->setValue(0);
        s_et_temp_ = new QSpinBox();
        s_et_temp_->setRange(0,50);
        s_et_temp_->setSingleStep(1);
        s_et_temp_->setValue(0);
        QVBoxLayout *vbox_group = new QVBoxLayout();
        QHBoxLayout *hbox_1 = new QHBoxLayout();
        QHBoxLayout *hbox_2 = new QHBoxLayout();
        QGroupBox *group_etalon_temp = new QGroupBox("Эталонные значения");
        QLabel *l_et_bar = new QLabel("Кол.точек температур");
        QLabel *l_et_temp = new QLabel("Кол.точек давлений");
        hbox_1->addWidget(l_et_bar);
        hbox_1->addWidget(s_et_bar_);
        hbox_1->setAlignment(Qt::AlignLeft);
        hbox_2->addWidget(l_et_temp);
        hbox_2->addWidget(s_et_temp_);
        hbox_2->setAlignment(Qt::AlignLeft);
        vbox_group->addLayout(hbox_1);
        vbox_group->addLayout(hbox_2);
        group_etalon_temp->setLayout(vbox_group);
        main_vbox->addWidget(tab_);
        main_vbox->addWidget(group_etalon_temp);
        main_vbox->addLayout(botton_hbox);
        window_c_p->setLayout(main_vbox);
        first_open_3 = true;
    }
    tab_->clear();
    for(auto& data : data_base_.GetDataSerACM()){
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
        data.model_temp = model_temp;
        data.model_bar = model_bar;
        QHBoxLayout *btn_hbox = new QHBoxLayout();
        QPushButton *btn_create_doc = new QPushButton("Создать отчет");
        connect(btn_create_doc, &QPushButton::clicked,this,[this,&data](){
            QString path = QApplication::applicationDirPath();
            QString path_doc = QFileDialog::getSaveFileName(nullptr, "Сохранить контрольные точки", path ,"*.txt");
            create_raport.CreateDoc(data,path_doc);
        });
        btn_hbox->addWidget(btn_create_doc);
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
    window_c_p->show();
}
void MainWindow::FilingTable(QStandardItemModel* model_temp,QStandardItemModel* model_bar){
    QVector<int> vec_bar = data_base_.GetCheckPointBar();
    QVector<double> vec_temp = data_base_.GetCheckPointTemp();
    model_temp->setColumnCount(s_et_temp_->value()+1);
    model_bar->setColumnCount(s_et_temp_->value()+1);
    model_temp->setRowCount(s_et_bar_->value()+1);
    model_bar->setRowCount(s_et_bar_->value()+1);
    model_bar->setItem(0,0,new QStandardItem("P/T"));
    model_temp->setItem(0,0,new QStandardItem("P/T"));
    for(int i = 1 ; i <= s_et_bar_->value();++i){
        QStandardItem *it = new QStandardItem(QString::number(vec_bar[i-1]));
        it->setTextAlignment(Qt::AlignCenter);
        model_bar->setItem(i,0,it);
        QStandardItem *it_b = new QStandardItem(QString::number(vec_bar[i-1]));
        it_b->setTextAlignment(Qt::AlignCenter);
        model_temp->setItem(i,0,it_b);
    }
    int index_temp = 0;
    for( int y = 1 ; y <= s_et_temp_->value();++y){
        QStandardItem *it_b = new QStandardItem(QString::number(vec_temp[index_temp]));
        it_b->setTextAlignment(Qt::AlignCenter);
        QStandardItem *it = new QStandardItem(QString::number(vec_temp[index_temp]));
        it->setTextAlignment(Qt::AlignCenter);
        model_bar->setItem(0,y,it);
        model_temp->setItem(0,y,it_b);
        index_temp += s_et_temp_->value();
    }
}
void MainWindow::FillAllTables(){
    for(DataSeriesACM& data : data_base_.GetDataSerACM()){
        FilingTable(data.model_temp,data.model_bar);
    }
    for(DataSeriesACM& data : data_base_.GetDataSerACM()){
        AnalisingSeries(data);
    }
}
void MainWindow::AnalisingSeries(DataSeriesACM data){
    const QVector<QDateTime> vec = data_base_.GetCheckPoints();

    int count = 0;
    int etalon_bar = s_et_bar_->value();
    int row = 1;
    int column = 1;

    for(QPointF& point : data.series_bar->points()){
        qint64 t = static_cast<qint64>(point.x());
        qint64 res = ((t + 500) / 1000) * 1000;
        QDateTime time = QDateTime::fromMSecsSinceEpoch(res);
        if(count == vec.size()){
            count = 0;
            row = 1;
            column = 1;
            break;
        }

        if(time == vec[count]){
            count++;
            QStandardItem *it = new QStandardItem(QString::number(point.y()));
            it->setTextAlignment(Qt::AlignCenter);
            data.model_bar->setItem(row,column,it);
            row++;
        }
        if(row > etalon_bar){
            row = 1;
            column++;
        }
    }
    for(QPointF& point : data.series_temp->points()){
        qint64 t = static_cast<qint64>(point.x());
        qint64 res = ((t + 500) / 1000) * 1000;
        QDateTime time = QDateTime::fromMSecsSinceEpoch(res);
        if(count == vec.size()){
            count = 0;
            break;
        }
        if(time == vec[count]){
            count++;
            QStandardItem *it = new QStandardItem(QString::number(point.y()));
            it->setTextAlignment(Qt::AlignCenter);
            data.model_temp->setItem(row,column,it);
            row++;
        }
        if(row > etalon_bar){
            row = 1;
            column++;
        }
    }
}
void MainWindow::CreateAllDoc(){
    create_raport.CreateAllDoc(data_base_.GetDataSerACM());
}
