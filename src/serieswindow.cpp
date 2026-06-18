#include "serieswindow.h"
#include "data_base.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPointer>

SeriesWindow::SeriesWindow(DataBase& data_base, QWidget* parent)
    : QWidget(parent), data_base_(data_base){
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    setWindowTitle("Настройка кривых");
}

void SeriesWindow::Refresh(){
    if(built_){
        return;
    }
    if(data_base_.GetDataSerACM().isEmpty()){
        return;
    }
    QVBoxLayout *vbox = new QVBoxLayout(this);
    for(auto& data : data_base_.GetDataSerACM()){
        QVector<Canal>& acm = data.vec_canal;
        if(acm.isEmpty()){
            return;
        }
        for(Canal& acm_unit : acm){
            QHBoxLayout *hbox = new QHBoxLayout();
            QPointer<QCheckBox> check = acm_unit.check_box;
            QLabel *label = new QLabel(acm_unit.name_canal +" "+ data.label_sensor->text());
            hbox->addWidget(check);
            hbox->addWidget(label);
            vbox->addLayout(hbox);
        }
    }
    built_ = true;
}
