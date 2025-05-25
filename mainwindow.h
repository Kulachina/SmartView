#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QCheckBox>
#include <QMessageBox>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QTabWidget>
#include <QGroupBox>
#include <QPushButton>
#include "chartview.h"
#include "data_base.h"
#include "dowland_file.h"
#include "qspinbox.h"
#include "createraport.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QMainWindow *parent = nullptr);
    ~MainWindow();
    void SetWindow();
public slots:
    void LoadDocumentEtalon();
    void LoadDocumentACM();
    void ToggledLegendPanel();
    void ShiftSeries();
    void ShiftLineinMouse();
    void WindowAxis();
    void WindowSeries();
    void WindowCheckPoint();
    void FillAllTables();
    void CreateAllDoc();
private:
    void closeEvent(QCloseEvent *event) override;
    void FilingTable(QStandardItemModel* model_temp, QStandardItemModel* model_bar);
    void AnalisingSeries(DataSeriesACM data);
    DataBase data_base_;
    CreateRaport create_raport;
    ChartView* chart_view_;
    DowlandFile dow_file_;
    QSpinBox *s_et_bar_;
    QSpinBox *s_et_temp_;
    QWidget *window_axes_,
            *window_series_,
            *window_c_p;
    QAction *load_doc_2,
            *toogled_legend,
            *shift_series,
            *data_in_time,
            *window_axis,
            *window_series;

    bool first_open_ = false,
         first_open_2 = false,
         first_open_3 = false;


};
#endif // MAINWINDOW_H
