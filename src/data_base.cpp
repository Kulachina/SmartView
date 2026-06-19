#include "data_base.h"

DataBase::DataBase()
{
    data_etalon_.reserve(50);
    data_acm_.reserve(50);
}

QVector<DataSeriesSensor>& DataBase::GetDataSerACM(){
    return data_acm_;
}
QVector<DataSeriesEtalon>& DataBase::GetDataSerEtalon(){
    return data_etalon_;
}
QVector<CheckRange>& DataBase::GetCheckRanges(){
    return check_ranges_;
}
QVector<double> DataBase::CalibrationTemp(bool use_ranges){
    if(use_ranges){
        QVector<double> v;
        for(const CheckRange& r : check_ranges_){
            v.push_back(r.avg_temp);
        }
        return v;
    }
    return check_points_temp;
}
QVector<double> DataBase::CalibrationBar(bool use_ranges){
    if(use_ranges){
        QVector<double> v;
        for(const CheckRange& r : check_ranges_){
            v.push_back(r.avg_bar);
        }
        return v;
    }
    return check_point_bar;
}
void DataBase::AddDataSerEtalon(DataSeriesEtalon data){
    data_etalon_.push_back(data);
}
void DataBase::AddDataSerACM(DataSeriesSensor data){
    for(int i = 0; i < data.vec_canal.size();++i){
        if(!data.vec_canal[i].select_box){
            data.vec_canal.removeAt(i);
            i--;
        }
    }
    data_acm_.push_back(data);
}
void DataBase::AddLabelSensor(QLabel* sensor,QLabel* name){
    QPointer<QLabel> point = sensor;
    QPointer<QLabel> point_2 = name;
    vector_name_sensor_.push_back(point);
    vector_name_.push_back(point_2);
}
void DataBase::AddListAxis(QValueAxis *axis){
    QPointer<QValueAxis> point = axis;
    list_axis_y_.push_back(point);
}
QList<QPointer<QValueAxis>>& DataBase::GetListAxis(){
    return list_axis_y_;
}
void DataBase::AddCheckPoint(qint64 q,double temp, double bar){
    QDateTime time = QDateTime::fromMSecsSinceEpoch(q);
    check_points64_.push_back(q);
    check_points_.push_back(time);
    check_points_temp.push_back(temp);
    check_point_bar.push_back(bar);
}
void DataBase::CreatePointsDate(){
    for(int i = 0;i < check_points64_.size();++i){
        QDateTime time = QDateTime::fromMSecsSinceEpoch(check_points64_[i]);
        check_points_.push_back(time);
    }
}
QVector<qint64>& DataBase::GetCheckPoints64(){
    return check_points64_;
}
QVector<QDateTime>& DataBase::GetCheckPoints(){
    return check_points_;
}
QVector<double>& DataBase::GetCheckPointTemp(){
    return check_points_temp;
}
QVector<double>& DataBase::GetCheckPointBar(){
    return check_point_bar;
}
void DataBase::SetDefaultAxisX(QDateTime max, QDateTime min){
    axis_min_ = min;
    axis_max_ = max;
}
std::pair<QDateTime,QDateTime> DataBase::GetDefaultAxisX(){
    return {axis_min_,axis_max_};
}
void DataBase::AddDeltaVolData(QString name_sensor, QVector<double> delta_bar, QVector<double> volume_bar,QVector<double> delta_temp, QVector<double> volume_temp){
    if(!data_acm_.isEmpty()){
        for(DataSeriesSensor& data : data_acm_){
            if(data.name_sensor == name_sensor){
                if(data.vec_canal[0].name_canal == "Давление"){
                    data.vec_canal[0].check_points = volume_bar;
                    data.vec_canal[0].delta_points = delta_bar;
                    data.vec_canal[1].check_points = volume_temp;
                    data.vec_canal[1].delta_points = delta_temp;
                } else {
                    data.vec_canal[1].check_points = volume_bar;
                    data.vec_canal[1].delta_points = delta_bar;
                    data.vec_canal[0].check_points = volume_temp;
                    data.vec_canal[0].delta_points = delta_temp;
                }
            }
        }
    }
}
void DataBase::ClearAll(){
    check_points_.clear();
    check_points_temp.clear();
    check_point_bar.clear();
    check_ranges_.clear();
    map_data_sensor_.clear();
    data_etalon_.clear();
    list_axis_y_.clear();
    vector_name_.clear();
    vector_name_sensor_.clear();
}

bool DataBase::FindSensor(QString& name){
    for(const auto& data : data_acm_){
        if(data.name_sensor == name){
            return true;
        }
    }
    return false;
}
bool DataBase::FindCanal(const QString& sensor, const QString& canal){
    for(const auto& data : data_acm_){
        if(data.name_sensor == sensor){
            for(const auto& can : data.vec_canal){
                if(can.name_canal == canal){
                    return true;
                }
            }
        }
    }
    return false;
}
