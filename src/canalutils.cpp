#include "canalutils.h"
#include <QLayoutItem>

void DeleteCanal(Canal& a){
    delete a.check_box;
    // Канал, который не попал в легенду (не выбран пользователем или пришёл
    // из .smv до вызова SetCanal), не имеет hbox — виджеты удаляем поимённо.
    if(a.hbox){
        while(QLayoutItem* item = a.hbox->takeAt(0)){
            delete item->widget();
            delete item;
        }
        delete a.hbox;
        a.hbox = nullptr;
    } else {
        delete a.label_data;
        delete a.label_delta;
        delete a.label_name_canal;
        delete a.label_name_sensor;
    }
    delete a.label;
    delete a.model;
    delete a.series;
    delete a.axis_y_;
    a.label = nullptr;
    a.label_data = nullptr;
    a.label_delta = nullptr;
    a.model = nullptr;
    a.series = nullptr;
    a.axis_y_ = nullptr;
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

QVector<DataSeriesSensor> SnapshotSensors(const QVector<DataSeriesSensor>& all_sensors,
                                          const QVector<DataSeriesSensor>& model){
    QVector<DataSeriesSensor> snapshot = all_sensors;
    for(DataSeriesSensor& sensor : snapshot){
        for(Canal& canal : sensor.vec_canal){
            for(const DataSeriesSensor& d : model){
                if(d.name_sensor != sensor.name_sensor){
                    continue;
                }
                for(const Canal& c : d.vec_canal){
                    if(c.name_canal == canal.name_canal){
                        canal = c;
                    }
                }
            }
        }
    }
    for(const DataSeriesSensor& d : model){
        bool found = false;
        for(const DataSeriesSensor& s : snapshot){
            if(s.name_sensor == d.name_sensor){
                found = true;
                break;
            }
        }
        if(!found){
            snapshot.push_back(d);
        }
    }
    return snapshot;
}
