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
#include <QTabWidget>
#include <QGroupBox>
#include <QPushButton>
#include "chartview.h"
#include "data_base.h"
#include "dowland_file.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QMainWindow *parent = nullptr);
    ~MainWindow();
    void SetWindow();
public slots:
    void LoadDocument();
    void ToggledLegendPanel();
    void ShiftSeries();
    void ShiftLineinMouse();
    void WindowAxis();
    void WindowSeries();
    void WindowCheckPoint();

private:
    void closeEvent(QCloseEvent *event) override;
    DataBase data_base_;
    ChartView* chart_view_;
    DowlandFile dow_file_;
    QWidget *window_axes_,
            *window_series_,
            *window_c_p;
    bool first_open_ = false,
         first_open_2 = false,
         first_open_3 = false;


};
#endif // MAINWINDOW_H
