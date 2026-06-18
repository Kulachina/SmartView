#include "canalutils.h"

void DeleteCanal(Canal& a){
    delete a.check_box;
    delete a.hbox->itemAt(3)->widget();
    delete a.hbox->itemAt(2)->widget();
    delete a.hbox->itemAt(1)->widget();
    delete a.hbox->itemAt(0)->widget();
    delete a.hbox;
    delete a.label;
    delete a.model;
    delete a.series;
    delete a.axis_y_;
    a.label_name_canal = nullptr;
    a.label_name_sensor = nullptr;
}

void DeleteSens(DataSeriesSensor& data){
    QVector<Canal>& acm = data.vec_canal;
    for(Canal& a : acm){
        DeleteCanal(a);
    }
    delete data.label_sensor;
    //delete data.line;
}
