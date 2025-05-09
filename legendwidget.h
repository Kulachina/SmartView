#ifndef LEGENDWIDGET_H
#define LEGENDWIDGET_H

#include <QWidget>

class LegendWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LegendWidget(QWidget *parent = nullptr);

private:
    QWidget legend_panel_;


};

#endif // LEGENDWIDGET_H
