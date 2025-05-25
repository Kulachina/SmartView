#include "mainwindow.h"

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
    QAction *load_doc = new QAction("Открыть документ",tool_bar);
    connect(load_doc, &QAction::triggered, this,&MainWindow::LoadDocument);
    QAction *load_doc_2 = new QAction("Добавить документ",tool_bar);
    //connect(load_doc, &QAction::triggered, this,&MainWindow::LoadDocument);
    QAction *toogled_legend = new QAction("скрыть/показать панель легенд",tool_bar);
    connect(toogled_legend, &QAction::triggered, this,&MainWindow::ToggledLegendPanel);
    QAction *shift_series = new QAction("Сдвиг кривых",tool_bar);
    connect(shift_series, &QAction::triggered, this,&MainWindow::ShiftSeries);
    QAction *data_in_time = new QAction("Данные в точке",tool_bar);
    connect(data_in_time, &QAction::triggered, this,&MainWindow::ShiftLineinMouse);
    QAction *window_axis = new QAction("Окно осей",tool_bar);
    connect(window_axis, &QAction::triggered, this,&MainWindow::WindowAxis);
    QAction *window_series = new QAction("Окно кривых",tool_bar);
    connect(window_series, &QAction::triggered, this,&MainWindow::WindowSeries);

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
void MainWindow::LoadDocument(){
    QString path = QApplication::applicationDirPath();
    QString path_doc = QFileDialog::getOpenFileName(this, "Открытие файла", path ,"Формат SmartLog (*.sml);;Текстовый документ (*.txt)");
    if(path_doc.isEmpty()){
        return;
    }
    if(path_doc.endsWith(".txt", Qt::CaseInsensitive)){
        dow_file_.LoadDocACM(path_doc);
        chart_view_->PanelLegendACM();
    }
    if(path_doc.endsWith(".sml", Qt::CaseInsensitive)){
        dow_file_.LoadDocEtalon(path_doc);
        chart_view_->PanelLegendEtalon();
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
void MainWindow::ShiftLineinMouse(){
    chart_view_->ToogledFlagLineInMouse();
}
void MainWindow::WindowAxis(){
    if(!first_open_){
        window_axes_ = new QWidget();
        window_axes_->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
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
        QTabWidget *tab = new QTabWidget();
        for(const auto& data : data_base_.GetDataSerACM()){
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
            QHBoxLayout *btn_hbox = new QHBoxLayout();
            QPushButton *btn_create_doc = new QPushButton("Сделать отчет");
            btn_hbox->addWidget(btn_create_doc);
            table_temp->setModel(model_temp);
            table_bar->setModel(model_bar);
            vbox_temp->addWidget(table_temp);
            vbox_bar->addWidget(table_bar);
            group_temp->setLayout(vbox_temp);
            group_bar->setLayout(vbox_bar);
            table_hbox->addWidget(group_bar);
            table_hbox->addWidget(group_temp);
            vbox->addLayout(table_hbox);
            vbox->addLayout(btn_hbox);
            tab_sensor->setLayout(vbox);
            tab->addTab(tab_sensor,data.name_sensor);
        }
        main_vbox->addWidget(tab);
        window_c_p->setLayout(main_vbox);

        first_open_3 = true;
    }
    window_c_p->show();
}
