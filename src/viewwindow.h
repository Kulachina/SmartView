#ifndef VIEWWINDOW_H
#define VIEWWINDOW_H
#pragma once
#include <QWidget>

class DataBase;
class ChartView;

// Окно «Вид»: автомасштаб и форма отображения кривых (прямоугольники/треугольники).
class ViewWindow : public QWidget {
    Q_OBJECT
public:
    ViewWindow(DataBase& data_base, ChartView* chart_view, QWidget* parent = nullptr);

private:
    void ReplaceTriangle();
    void ReplaceRectangle();
    void AutoZoom();
    DataBase& data_base_;
    ChartView* chart_view_;
};

#endif // VIEWWINDOW_H
