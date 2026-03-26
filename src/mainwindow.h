#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma once
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
#include <QDoubleSpinBox>
#include <QListWidget>
#include "chartview.h"
#include "data_base.h"
#include "dowland_file.h"
#include "qspinbox.h"
#include "createraport.h"
#include "error_table.h"
#include "las.h"
#include "updater.h"

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
    void LoadDocumentAMT();
    void LoadDocumentLAS();
    void ToggledLegendPanel();
    void ShiftSeries();
    void ShiftCheckPoint();
    void ShiftLineinMouse();
    void WindowAxis();
    void WindowSeries();
    void WindowMasterPoint();
    void WindowCheckPoints();
    void WindowDeleteSensor();
    void WindowView();
    void FillFromOneTable();
    void FillAllTables();
    void FillAllTablesLAS();
    void CreateAllDoc();
    void CreateAllDocLAS();
    void ReplaceTriangle();
    void ReplaceRectangle();
    void WindowTableError();
    void AutoZoom();
    void SaveAllSV();
    void CheckUpdate();
private:
    QWidget* CreateTabMasterLAS();
    QWidget* CreateTabMasterACM();
    DataSeriesSensor* PreparationData();
    void UncheckCanals();
    void CheckCanals();
    void RebuildCanal(Canal& canal);
    void SetPanelLegend();
    void CreatePanelLegend();
    void ActoinWinSaC();
    void Clear();
    void CreateCheckPoint(QTime hour);
    void CreateInContentLAS();
    void CreateInContentACM();
    void WindowSensorAndCanal(QVector<DataSeriesSensor>& vec_data);
    void ChangeAllTables(QStandardItem * item);
    void DeleteCheckPoint();
    void closeEvent(QCloseEvent *event) override;
    void FilingTableLAS(QStandardItemModel* model);
    void FilingTable(QStandardItemModel* model_temp, QStandardItemModel* model_bar);
    void AnalisingSeries(DataSeriesSensor data);
    void AnalisingSeriesLAS(DataSeriesSensor data);
    void DeleteOneSens();
    void DeleteSens(DataSeriesSensor& data);
    void DeleteCanal(Canal& canal);
    void DeleteAllSens();
    void UpdateComboBox();
    void CreateTableCheckPoints();
    void ReplaceCheckSeries();
    DataBase data_base_;
    ChartView* chart_view_;
    DowlandFile dow_file_;
    CreateRaport create_raport_;
    ErrorTable error_table_;
    Updater update_;
    Las las_;
    QVector<DataSeriesSensor> data_sensor_;
    QVector<DataSeriesSensor> all_data_sensor_;
    QSpinBox *s_et_bar_,
             *s_et_temp_,
             *s_et_bar_las_,
             *s_et_temp_las_;
    QDoubleSpinBox *s_step_bar_,
                   *s_step_bar_las_;
    QVBoxLayout *vb_del_sens_,
                *vbox_axes_;
    QWidget *window_axes_,
            *window_series_,
            *window_c_p_,
            *window_del_sens_,
            *window_check_points_,
            *win_sens_can_,
            *window_view_;
    QAction *load_doc_2_,
            *load_doc_3_,
            *load_doc_4_,
            *toogled_legend_,
            *shift_series_,
            *shift_check_point_,
            *data_in_time_,
            *window_axis_,
            *action_series_,
            *delete_sensor_,
            *change_canal_;
    QTabWidget *tab_,
               *tab_las_;
    QCheckBox *check_step_bar_,
              *check_step_bar_las_,
              *ch_sel_1_table_,
              *ch_sel_1_table_las_;
    QComboBox *combo_del_sens_;
    QStringList path_doc_;
    QString  save_path_,
             name_canal_1_,
             name_canal_2_,
             name_canal_las_1_,
             name_canal_las_2_;
    QStandardItemModel *fix_model_;
    QTableView *fix_table_;
    QListWidget *sensor_list_;
    QListWidget *canal_list_;
    QStringList chanels_ = {"GK-ГК","Q-Расход","Т-Температура","E-Термокомпенсация","Р-Давление","VLG-Влагомер","REZ-Резистивиметр","PL-Плотность флюида"};
    bool first_open_ = false,
         first_open_2_ = false,
         first_open_3_ = false,
         first_open_4_ = false,
         first_open_doc_ = false,
         first_open_etalon_ = false,
         first_open_win_check_points_ = false,
         first_open_win_view_ = false,
         create_axis_ = false,
        create_sens_select_ = true,
        first_open_win_sens_can_ = true,
        foo_call_ = false,
        flag_termocompens_ = false,
        change_all_tables_ = false;
};
#endif // MAINWINDOW_H
