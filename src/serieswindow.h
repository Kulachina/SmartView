#ifndef SERIESWINDOW_H
#define SERIESWINDOW_H
#pragma once
#include <QWidget>

class DataBase;

// Окно «Настройка кривых»: список каналов ACM с их чекбоксами активности.
class SeriesWindow : public QWidget {
    Q_OBJECT
public:
    explicit SeriesWindow(DataBase& data_base, QWidget* parent = nullptr);
    void Refresh();   // построить содержимое (один раз) перед показом

private:
    DataBase& data_base_;
    bool built_ = false;
};

#endif // SERIESWINDOW_H
