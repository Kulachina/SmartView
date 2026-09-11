#include "mainwindow.h"
#include "canalutils.h"
#include "chartoverview.h"
#include "checkpointswindow.h"
#include "rangeswindow.h"
#include "axiswindow.h"
#include "serieswindow.h"
#include "viewwindow.h"
#include "masterpointswindow.h"
#include "deletesensorwindow.h"
#include "sensorcanaleditor.h"
#include "documentloader.h"
#include "aboutdialog.h"
#include <QMenuBar>
#include <QMenu>
#include <QGridLayout>
#include <QIcon>
#include <QToolBar>
#include <QAction>
#include <QHBoxLayout>
#include <QWidget>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QCloseEvent>

namespace {
// Кнопка панели инструментов: иконка из ресурсов плюс подсказка при наведении.
// Панель показывает только иконки, поэтому название действия живёт в подсказке.
void SetToolButton(QAction *action, const QString &icon, const QString &tip){
    action->setIcon(QIcon(":/icons/" + icon + ".svg"));
    action->setToolTip(tip);
}
}

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
    QAction *open = new QAction("Открыть");
    connect(open, &QAction::triggered, this, &MainWindow::OpenDocument);
    menu_file->addAction(open);
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
    connect(close, &QAction::triggered, this, &MainWindow::close);
    menu_file->addAction(close);
    QAction *report = new QAction("Отчеты");
    QMenu *menu_report = new QMenu();
    report->setMenu(menu_report);
    QAction *export_protocol = new QAction("Протокол калибровки");
    connect(export_protocol, &QAction::triggered, this, &MainWindow::ExportProtocolTemplate);
    menu_report->addAction(export_protocol);
    QAction *master_point = new QAction("Мастер точек");
    connect(master_point, &QAction::triggered, this,&MainWindow::WindowMasterPoint);
    QAction *view = new QAction("Вид");
    connect(view, &QAction::triggered, this,&MainWindow::WindowView);
    QAction *check_point = new QAction("Контрольные точки");
    connect(check_point, &QAction::triggered, this,&MainWindow::WindowCheckPoints);
    QAction *check_range = new QAction("Контрольные диапазоны");
    connect(check_range, &QAction::triggered, this,&MainWindow::WindowRanges);
    QAction *error_delta = new QAction("Таблица погрешностей");
    connect(error_delta, &QAction::triggered, this,&MainWindow::WindowTableError);
    menu->addAction(file);
    menu->addAction(report);
    menu->addAction(view);
    menu->addAction(check_point);
    menu->addAction(check_range);
    menu->addAction(master_point);
    menu->addAction(error_delta);
    QAction *help = new QAction("Справка");
    QMenu *menu_help = new QMenu();
    help->setMenu(menu_help);
    QAction *about = new QAction("О программе");
    connect(about, &QAction::triggered, this, &MainWindow::WindowAbout);
    menu_help->addAction(about);
    QAction *update = new QAction("Проверить обновление");
    connect(update, &QAction::triggered, this, &MainWindow::CheckUpdate);
    menu_help->addAction(update);
    menu->addAction(help);
    chart_view_ = new ChartView(nullptr, data_base_);
    chart_overview_ = new ChartOverview(data_base_, chart_view_->GetAxisX());
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
    select_range_ = new QAction("Выделение диапазона");
    select_range_->setCheckable(true);
    select_range_->setEnabled(false);
    connect(select_range_, &QAction::triggered, this, [this](){ chart_view_->ToogledFlagSelectRange(); });
    QAction *load_doc = new QAction("Открыть Эталон",tool_bar);
    connect(load_doc, &QAction::triggered, this,&MainWindow::LoadDocumentEtalon);
    change_canal_ = new QAction("Редактировать каналы",tool_bar);
    change_canal_->setEnabled(false);
    connect(change_canal_, &QAction::triggered, this,&MainWindow::ActoinWinSaC);
    SetToolButton(load_doc, "open_etalon", "Открыть Эталон");
    SetToolButton(load_doc_2_, "add_las", "Добавить прибор из LAS");
    SetToolButton(load_doc_3_, "add_acm", "Добавить прибор АЦМ");
    SetToolButton(load_doc_4_, "add_amt", "Добавить прибор АМТ");
    SetToolButton(change_canal_, "edit_canals", "Редактировать каналы");
    SetToolButton(window_axis_, "axes", "Окно осей");
    SetToolButton(toogled_legend_, "legend_panel", "Скрыть/показать панель легенд");
    SetToolButton(shift_series_, "shift_series", "Сдвиг кривых");
    SetToolButton(shift_check_point_, "shift_checkpoint", "Сдвиг контрольных точек");
    SetToolButton(select_range_, "select_range", "Выделение диапазона");
    SetToolButton(data_in_time_, "data_in_point", "Данные в точке");
    SetToolButton(delete_sensor_, "delete_sensor", "Удалить прибор");
    tool_bar->setIconSize(QSize(24,24));
    tool_bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tool_bar->addAction(load_doc);
    tool_bar->addAction(load_doc_2_);
    tool_bar->addAction(load_doc_3_);
    tool_bar->addAction(load_doc_4_);
    tool_bar->addSeparator();
    tool_bar->addAction(change_canal_);
    tool_bar->addAction(window_axis_);
    tool_bar->addAction(toogled_legend_);
    tool_bar->addSeparator();
    tool_bar->addAction(shift_series_);
    tool_bar->addAction(shift_check_point_);
    tool_bar->addAction(select_range_);
    tool_bar->addAction(data_in_time_);
    tool_bar->addSeparator();
    tool_bar->addAction(delete_sensor_);

    addToolBar(tool_bar);
    SetWindow();
    // Карта <-> ось времени основного графика. Обратная сторона — сигнал самой
    // оси, поэтому любой зум (рамкой, откатом, загрузкой) карта видит сама.
    connect(chart_view_->GetAxisX(), &QDateTimeAxis::rangeChanged,
            chart_overview_, &ChartOverview::OnViewRangeChanged);
    connect(chart_overview_, &ChartOverview::ZoomAboutToChange,
            chart_view_, &ChartView::PushZoomState);
    connect(chart_view_, &ChartView::CursorTimeChanged,
            chart_overview_, &ChartOverview::SetCursorTime);
    connect(chart_view_, &ChartView::CursorLeftChart,
            chart_overview_, &ChartOverview::ClearCursorTime);
    dow_file_.SetChartDoc(chart_view_->GetChart(),chart_view_->GetAxisTemp(),chart_view_->GetAxisBar());
    dow_file_.SetAxisTime(chart_view_->GetAxisX());
    check_points_window_ = new CheckPointsWindow(data_base_, chart_view_);
    connect(chart_view_, &ChartView::AddCheckPointRequested,
            check_points_window_, &CheckPointsWindow::AddCheckPointAt);
    connect(chart_view_, &ChartView::DeleteCheckPointRequested,
            check_points_window_, &CheckPointsWindow::DeleteCheckPointAtTime);
    ranges_window_ = new RangesWindow(data_base_, chart_view_);
    connect(chart_view_, &ChartView::AddRangeRequested,
            ranges_window_, &RangesWindow::AddRangeAt);
    axis_window_ = new AxisWindow(data_base_);
    series_window_ = new SeriesWindow(data_base_);
    view_window_ = new ViewWindow(data_base_, chart_view_);
    master_window_ = new MasterPointsWindow(data_base_, create_raport_);
    delete_window_ = new DeleteSensorWindow(data_base_, all_data_sensor_);
    sensor_editor_ = new SensorCanalEditor(data_base_, chart_view_, data_sensor_, all_data_sensor_);
    protokol_writer_ = new ProtocolWriter(data_base_);
    loader_ = new DocumentLoader(data_base_, dow_file_, las_, chart_view_, all_data_sensor_, this);
}
MainWindow::~MainWindow()
{

}
void MainWindow::SetWindow(){
    // Карта графика — узкая полоса над графиком; панель легенд занимает правую
    // колонку на всю высоту, как и раньше.
    QGridLayout *grid = new QGridLayout();
    grid->addWidget(chart_overview_,0,0);
    grid->addWidget(chart_view_,1,0);
    grid->addWidget(chart_view_->GetWidgetLegend(),0,1,2,1);
    grid->setColumnStretch(0,8);
    grid->setColumnStretch(1,2);
    grid->setRowStretch(0,0);
    grid->setRowStretch(1,1);
    QWidget *w = new QWidget();
    w->setLayout(grid);
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
        EnableDocumentActions();
        chart_overview_->Rebuild();
        first_open_etalon_ = true;
    } else {
        int reply = QMessageBox::question(this, "Новый Эталон", "Вы уверены что хоите открыть новый Эталон и потеряете текущий прогресс?",QMessageBox::Yes | QMessageBox::No);
        if(reply == QMessageBox::Yes){
            DeleteAllSens();
            loader_->LoadEtalonReplace();
            chart_overview_->Rebuild();
        }
    }
}
void MainWindow::ToggledLegendPanel(){
    QWidget *legend = chart_view_->GetWidgetLegend();
    const bool hide = legend->isVisible();
    legend->setVisible(!hide);
    // QGridLayout держит за колонкой её долю растяжения, даже когда
    // единственный виджет в колонке скрыт: без обнуления график не займёт
    // освободившееся место.
    if(QGridLayout *grid = qobject_cast<QGridLayout*>(centralWidget()->layout())){
        grid->setColumnStretch(1, hide ? 0 : 2);
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
    // Копии тех же приборов: виджеты уже удалены выше, оставлять их нельзя.
    all_data_sensor_.clear();
    data_sensor_.clear();
    chart_view_->ClearPanelLegend();
    chart_overview_->Clear();
    data_base_.ClearAll();
    dow_file_.ClearAll();
}
void MainWindow::WindowCheckPoints(){
    check_points_window_->Refresh();
    check_points_window_->show();
}
void MainWindow::WindowRanges(){
    ranges_window_->Refresh();
    ranges_window_->show();
}
void MainWindow::WindowView(){
    view_window_->show();
}
void MainWindow::EnableDocumentActions(){
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
    select_range_->setEnabled(true);
}
void MainWindow::OpenDocument(){
    if(first_open_etalon_){
        int reply = QMessageBox::question(this, "Открыть документ",
                                          "Открыть другой документ? Текущий прогресс будет потерян.",
                                          QMessageBox::Yes | QMessageBox::No);
        if(reply != QMessageBox::Yes){
            return;
        }
        DeleteAllSens();
    }
    if(!loader_->OpenDocument()){
        return;
    }
    EnableDocumentActions();
    chart_overview_->Rebuild();
    first_open_etalon_ = true;
}
void MainWindow::SaveAllSV(){
    if(data_base_.GetDataSerEtalon().isEmpty()){
        QMessageBox::information(this, "Сохранение", "Документ пуст: сначала откройте эталон.");
        return;
    }
    chart_view_->ZeroZoom();
    QString path = QApplication::applicationDirPath();
    QString path_doc = QFileDialog::getSaveFileName(nullptr, "Сохранить данные", path ,"Формат SmartView (*.smv)");
    if(path_doc.isEmpty()){
        return;
    }
    if(!path_doc.endsWith(".smv", Qt::CaseInsensitive)){
        path_doc += ".smv";
    }
    dow_file_.SaveSVDoc(path_doc, SnapshotSensors(all_data_sensor_, data_base_.GetDataSerACM()));
}
void MainWindow::CheckUpdate(){
    update_.ManualCheck();
}
void MainWindow::WindowAbout(){
    AboutDialog(this).exec();
}
void MainWindow::ExportProtocolTemplate(){
    protokol_writer_->GetSpisokPribors();
    protokol_writer_->show();
}
