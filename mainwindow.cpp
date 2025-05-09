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
    menu->addAction(file);
    menu->addAction(report);
    menu->addAction(master_point);
    chart_view_ = new ChartView(nullptr, data_base_);
    QToolBar *tool_bar = new QToolBar();
    QAction *load_doc = new QAction("load_doc",tool_bar);
    connect(load_doc, &QAction::triggered, this,&MainWindow::LoadDocument);
    tool_bar->addAction(load_doc);
    addToolBar(tool_bar);
    SetWindow();
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
    if(path_doc.endsWith(".sml", Qt::CaseInsensitive)){
        dow_file_.SetChartDoc(chart_view_->GetChart());
        dow_file_.SetAxisTime(chart_view_->GetAxisX());
        dow_file_.LoadDocumentData(path_doc);

    }
}

