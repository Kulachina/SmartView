#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma once
#include <QMainWindow>
#include <QAction>
#include "chartview.h"
#include "data_base.h"
#include "dowland_file.h"
#include "createraport.h"
#include "error_table.h"
#include "las.h"
#include "updater.h"

class CheckPointsWindow;
class RangesWindow;
class AxisWindow;
class SeriesWindow;
class ViewWindow;
class MasterPointsWindow;
class DeleteSensorWindow;
class SensorCanalEditor;
class DocumentLoader;

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
    void WindowRanges();
    void WindowDeleteSensor();
    void WindowView();
    void WindowTableError();
    void SaveAllSV();
    void CheckUpdate();
private:
    void ActoinWinSaC();
    void closeEvent(QCloseEvent *event) override;
    void DeleteAllSens();
    DataBase data_base_;
    ChartView* chart_view_;
    DowlandFile dow_file_;
    CreateRaport create_raport_;
    ErrorTable error_table_;
    Updater update_;
    Las las_;
    QVector<DataSeriesSensor> data_sensor_;
    QVector<DataSeriesSensor> all_data_sensor_;
    CheckPointsWindow* check_points_window_;
    RangesWindow* ranges_window_;
    AxisWindow* axis_window_;
    SeriesWindow* series_window_;
    ViewWindow* view_window_;
    MasterPointsWindow* master_window_;
    DeleteSensorWindow* delete_window_;
    SensorCanalEditor* sensor_editor_;
    DocumentLoader* loader_;
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
            *change_canal_,
            *select_range_;
    bool first_open_etalon_ = false;
};
#endif // MAINWINDOW_H
