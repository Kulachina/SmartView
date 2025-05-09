#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QFileDialog>
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


private:
    DataBase data_base_;
    ChartView* chart_view_;
    DowlandFile dow_file_;


};
#endif // MAINWINDOW_H
