#include "deletesensorwindow.h"
#include "data_base.h"
#include "canalutils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

DeleteSensorWindow::DeleteSensorWindow(DataBase& data_base, QVector<DataSeriesSensor>& all_data_sensor, QWidget* parent)
    : QWidget(parent), data_base_(data_base), all_data_sensor_(all_data_sensor){
    setWindowTitle("Удалить прибор");
    QVBoxLayout *vb = new QVBoxLayout(this);
    combo_del_sens_ = new QComboBox();
    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(new QLabel("Выберите прибор"));
    hbox->addWidget(combo_del_sens_);
    QHBoxLayout *hbox_btn = new QHBoxLayout();
    QPushButton *btn_del = new QPushButton("Удалить");
    connect(btn_del, &QPushButton::clicked, this, &DeleteSensorWindow::DeleteSelected);
    QPushButton *btn_cancel = new QPushButton("Отмена");
    connect(btn_cancel, &QPushButton::clicked, this, &QWidget::hide);
    hbox_btn->addWidget(btn_del,0,Qt::AlignLeft);
    hbox_btn->addWidget(btn_cancel,0,Qt::AlignRight);
    vb->addLayout(hbox);
    vb->addLayout(hbox_btn);
}

void DeleteSensorWindow::Refresh(){
    QStringList sensors;
    for(const DataSeriesSensor& data : data_base_.GetDataSerACM()){
        sensors << data.name_sensor;
    }
    combo_del_sens_->clear();
    combo_del_sens_->addItems(sensors);
}

void DeleteSensorWindow::DeleteSelected(){
    QString sens = combo_del_sens_->currentText();
    QVector<DataSeriesSensor>& data = data_base_.GetDataSerACM();
    for(auto it = data.begin();it != data.end();){
        if(sens == it->name_sensor){
            DeleteSens(*it);
            it = data.erase(it);
            Refresh();
            break;
        } else {
            ++it;
        }
    }
    for(auto it = all_data_sensor_.begin();it != all_data_sensor_.end();){
        if(sens == it->name_sensor){
            //DeleteSens(*it);
            it = all_data_sensor_.erase(it);
            Refresh();
            return;
        } else {
            ++it;
        }
    }
}
