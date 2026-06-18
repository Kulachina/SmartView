#include "mainwindow.h"
#include "canalutils.h"
#include "checkpointswindow.h"
#include "axiswindow.h"
#include "serieswindow.h"
#include "viewwindow.h"
#include "masterpointswindow.h"
#include "deletesensorwindow.h"
#include "sensorcanaleditor.h"
#include "documentloader.h"
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QHBoxLayout>
#include <QWidget>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent),
    data_base_(),
    dow_file_(data_base_),
    create_raport_(data_base_),
    error_table_(data_base_,create_raport_,nullptr),
    update_()
{
    update_.AutoCheck();
    QMenuBar *menu = menuBar();
    QMenu *menu_file = new QMenu();
    QAction *file = new QAction("Файл");
    file->setMenu(menu_file);
    QAction *import = new QAction("Импорт");
    QAction *update = new QAction("Проверить обновление");
    menu_file->addAction(import);
    menu_file->addAction(update);
    connect(update, &QAction::triggered,this, &MainWindow::CheckUpdate);
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
    shift_check_point_->setCheckable(true);
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
    check_points_window_ = new CheckPointsWindow(data_base_, chart_view_);
    connect(chart_view_, &ChartView::AddCheckPointRequested,
            check_points_window_, &CheckPointsWindow::AddCheckPointAt);
    axis_window_ = new AxisWindow(data_base_);
    series_window_ = new SeriesWindow(data_base_);
    view_window_ = new ViewWindow(data_base_, chart_view_);
    master_window_ = new MasterPointsWindow(data_base_, create_raport_);
    delete_window_ = new DeleteSensorWindow(data_base_, all_data_sensor_);
    sensor_editor_ = new SensorCanalEditor(data_base_, chart_view_, data_sensor_, all_data_sensor_);
    loader_ = new DocumentLoader(data_base_, dow_file_, las_, chart_view_, this);
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
    for(DataSeriesSensor& data : loader_->LoadACM())
        data_sensor_.push_back(data);
    if(!data_sensor_.isEmpty()){
        sensor_editor_->OpenForNew();
    }
}
void MainWindow::LoadDocumentLAS(){
    for(DataSeriesSensor& data : loader_->LoadLAS())
        data_sensor_.push_back(data);
    if(!data_sensor_.isEmpty()){
        sensor_editor_->OpenForNew();
    }
}
void MainWindow::LoadDocumentAMT(){
    for(DataSeriesSensor& data : loader_->LoadAMT())
        data_sensor_.push_back(data);
    if(!data_sensor_.isEmpty()){
        sensor_editor_->OpenForNew();
    }
}
void MainWindow::ActoinWinSaC(){
    if(!all_data_sensor_.isEmpty()){
        sensor_editor_->OpenForEdit();
    }
}
void MainWindow::LoadDocumentEtalon(){
    if(!first_open_etalon_){
        if(!loader_->LoadEtalonInitial()){
            return;
        }
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
        change_canal_->setEnabled(true);
        first_open_etalon_ = true;
    } else {
        int reply = QMessageBox::question(this, "Новый Эталон", "Вы уверены что хоите открыть новый Эталон и потеряете текущий прогресс?",QMessageBox::Yes | QMessageBox::No);
        if(reply == QMessageBox::Yes){
            DeleteAllSens();
            loader_->LoadEtalonReplace();
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
    axis_window_->Refresh();
    axis_window_->show();
}
void MainWindow::WindowSeries(){
    if(data_base_.GetDataSerACM().isEmpty()){
        return;
    }
    series_window_->Refresh();
    series_window_->show();
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
    master_window_->Open();
}
void MainWindow::WindowDeleteSensor(){
    delete_window_->Refresh();
    delete_window_->show();
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
    dat.clear();
    chart_view_->ClearPanelLegend();
    data_base_.ClearAll();
    dow_file_.ClearAll();
}
void MainWindow::WindowCheckPoints(){
    check_points_window_->Refresh();
    check_points_window_->show();
}
void MainWindow::WindowView(){
    view_window_->show();
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
void MainWindow::CheckUpdate(){
    update_.ManualCheck();
}
