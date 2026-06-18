#ifndef RANGESWINDOW_H
#define RANGESWINDOW_H
#pragma once
#include <QWidget>
#include <QTime>

class DataBase;
class ChartView;
class QTableView;
class QStandardItemModel;

// Окно «Контрольные диапазоны»: ввод отрезка [t1,t2], усреднение эталонных
// кривых на нём и таблица сохранённых диапазонов (аналог окна КТ).
class RangesWindow : public QWidget {
    Q_OBJECT
public:
    RangesWindow(DataBase& data_base, ChartView* chart_view, QWidget* parent = nullptr);
    void Refresh();   // перестроить таблицу диапазонов перед показом

public slots:
    // Добавить диапазон по [t1,t2], перерисовать оверлей и таблицу.
    // Вызывается из окна (кнопка) и из ChartView (выделение мышью).
    void AddRangeAt(QTime t1, QTime t2);

private:
    void AddRange(QTime t1, QTime t2);
    void DeleteSelected();
    DataBase& data_base_;
    ChartView* chart_view_;
    QTableView* table_;
    QStandardItemModel* model_;
};

#endif // RANGESWINDOW_H
