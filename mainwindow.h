#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QCheckBox>

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

private:
    DataBase data_base_;
    ChartView* chart_view_;
    DowlandFile dow_file_;
    QWidget *window_axes_,
            *window_series_;
    bool first_open_ = false,
         first_open_2 = false;


};
#endif // MAINWINDOW_H
