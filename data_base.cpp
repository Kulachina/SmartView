#include "data_base.h"

DataBase::DataBase()
{
    data_etalon_.reserve(50);
    data_acm_.reserve(50);
}

QVector<DataSeriesACM>& DataBase::GetDataSerACM(){
    return data_acm_;
}
QVector<DataSeriesEtalon>& DataBase::GetDataSerEtalon(){
    return data_etalon_;
}
void DataBase::AddDataSerEtalon(DataSeriesEtalon data){
    data_etalon_.push_back(data);
}
void DataBase::AddDataSerACM(DataSeriesACM& data){
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
void DataBase::AddCheckPoint(qint64 q,double temp, int bar){
    QDateTime time = QDateTime::fromMSecsSinceEpoch(q);
    check_points_.push_back(time);
    check_points_temp.push_back(temp);
    check_point_bar.push_back(bar);
}
const QVector<QDateTime>& DataBase::GetCheckPoints(){
    return check_points_;
}
QVector<double>& DataBase::GetCheckPointTemp(){
    return check_points_temp;
}
QVector<int>& DataBase::GetCheckPointBar(){
    return check_point_bar;
}
void DataBase::SetDefaultAxisX(QDateTime max, QDateTime min){
    axis_min_ = min;
    axis_max_ = max;
}
std::pair<QDateTime,QDateTime> DataBase::GetDefaultAxisX(){
    return {axis_min_,axis_max_};
}
