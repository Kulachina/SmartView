#include "axiswindow.h"
#include "data_base.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>

AxisWindow::AxisWindow(DataBase& data_base, QWidget* parent)
    : QWidget(parent), data_base_(data_base){
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    setWindowTitle("Настройка осей");
    vbox_axes_ = new QVBoxLayout(this);
}

void AxisWindow::Refresh(){
    while(QLayoutItem *item = vbox_axes_->itemAt(0)){
        if(QLayout *l = item->layout()){
            while(QLayoutItem *it = l->itemAt(0)){
                if(QWidget *w = it->widget()){
                    delete w;
                }
            }
            delete l;
        }
    }
    for(auto& axis : data_base_.GetListAxis()){
        QHBoxLayout *hbox = new QHBoxLayout();
        QCheckBox *check = new QCheckBox();
        if(!axis.isNull()){
            check->setCheckState(axis->isVisible() ? Qt::Checked : Qt::Unchecked);
            hbox->addWidget(check);
            hbox->addWidget(new QLabel(axis->titleText()));
            vbox_axes_->addLayout(hbox);
            connect(check, &QCheckBox::toggled, this,[axis,check](){
                axis->setVisible(check->isChecked());
            });
        }
    }
}
