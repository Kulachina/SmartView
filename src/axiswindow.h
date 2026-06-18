#ifndef AXISWINDOW_H
#define AXISWINDOW_H
#pragma once
#include <QWidget>

class DataBase;
class QVBoxLayout;

// Окно «Настройка осей»: список осей Y с чекбоксами видимости.
class AxisWindow : public QWidget {
    Q_OBJECT
public:
    explicit AxisWindow(DataBase& data_base, QWidget* parent = nullptr);
    void Refresh();   // перестроить список осей перед показом

private:
    DataBase& data_base_;
    QVBoxLayout* vbox_axes_;
};

#endif // AXISWINDOW_H
