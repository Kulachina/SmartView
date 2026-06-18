#ifndef CHECKPOINTSWINDOW_H
#define CHECKPOINTSWINDOW_H
#pragma once
#include <QWidget>
#include <QTime>

class DataBase;
class ChartView;
class QTableView;
class QStandardItemModel;

// Окно «Контрольные точки»: добавление/удаление КТ и таблица их значений.
class CheckPointsWindow : public QWidget {
    Q_OBJECT
public:
    CheckPointsWindow(DataBase& data_base, ChartView* chart_view, QWidget* parent = nullptr);
    void Refresh();   // перестроить таблицу КТ перед показом

public slots:
    // Добавить КТ по времени, перестроить серию точек и таблицу.
    // Вызывается из окна (кнопка) и из ChartView (правый клик по графику).
    void AddCheckPointAt(QTime time_read);
    // Удалить КТ с заданным временем (правый клик по маркеру на графике).
    void DeleteCheckPointAtTime(QTime time);

private:
    void AddCheckPoint(QTime time_read);
    void DeleteSelected();
    DataBase& data_base_;
    ChartView* chart_view_;
    QTableView* fix_table_;
    QStandardItemModel* fix_model_;
};

#endif // CHECKPOINTSWINDOW_H
