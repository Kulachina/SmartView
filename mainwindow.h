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
#include <QComboBox>
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
    void WindowDeleteSensor();
    void FillAllTables();
    void CreateAllDoc();
private:
    void closeEvent(QCloseEvent *event) override;
    void FilingTable(QStandardItemModel* model_temp, QStandardItemModel* model_bar);
    void AnalisingSeries(DataSeriesACM data);
    void DeleteSens();
    DataBase data_base_;
    CreateRaport create_raport;
    ChartView* chart_view_;
    DowlandFile dow_file_;
    QSpinBox *s_et_bar_,
             *s_et_temp_;
    QVBoxLayout *vb_del_sens_;
    QWidget *window_axes_,
            *window_series_,
            *window_c_p,
            *window_del_sens_;
    QAction *load_doc_2_,
            *toogled_legend_,
            *shift_series_,
            *data_in_time_,
            *window_axis_,
            *action_series_,
            *delete_sensor_;
    QTabWidget *tab_;
    QComboBox *combo_del_sens_;
    bool first_open_ = false,
         first_open_2_ = false,
         first_open_3_ = false,
         first_open_4_ = false;


};
#endif // MAINWINDOW_H
