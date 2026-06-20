#include "las.h"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextCodec>


Las::Las()
{
    prog_ = new QProgressBar();
}

DataSeriesSensor& Las::DowlandLas(const QString path){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){
        qWarning() << "Не удалось открыть файл для чтения:" << path;
        return data_;
    }
    data_.vec_canal.clear();
    QTextStream in(&file);
    QTextCodec *codec = QTextCodec::codecForName("IBM 866");
    QByteArray ba = file.readAll();
    QString all_string =  codec->toUnicode(ba);
    QStringList all = all_string.split("\n",Qt::SkipEmptyParts);
    file.close();
    QStringList words;
    int size = all.size();
    prog_->setWindowFlags(Qt::Window | Qt::WindowTitleHint);
    prog_->setWindowTitle("Загрузка");
    prog_->setRange(0,size);
    prog_->setValue(0);
    prog_->show();
    for(int i = 0; i < size;++i){
        prog_->setValue(i);
        QCoreApplication::processEvents();
        words = all[i].split(" ",Qt::SkipEmptyParts);
        if(words.isEmpty()){
            continue;
        }
        if(words[0] == "DATE."){
            SetStartDate(words);
        }
        if(words[0] == "TIME."){
            SetStartTime(words);
        }
        if(select_curve_){
            SelectCurve(words);
        }
        if(words[0] == "~CURVE"){
            select_curve_ = true;
        }
        if(words[0] == "IPRB."){
            SetNameSensor(words);
        }
        if(words[0] == "NPRB."){
            SetNumSensor(words);
        }
        if(log_data_){
            LogData(words);
        }
        if(!select_curve_ && words[0] == "~ASCII"){
            log_data_ = true;
        }
    }
    log_data_= false;
    prog_->hide();
    return data_;
}
void Las::SetStartDate(const QStringList& words){
    int index = words[1].indexOf(':');
    QString word = words[1].left(index);
    date_ = word.replace('.','/');
}
void Las::SetStartTime(const QStringList& words){

}
void Las::SelectCurve(QStringList& words){
    if(words[0].contains("~",Qt::CaseInsensitive)){
        select_curve_ = false;
        CreateSensor();
        name_curve_.clear();
        return;
    }
    name_curve_.push_back(words[0]);
}

void Las::LogData(const QStringList& words){
    QVector<Canal>& vec_canal = data_.vec_canal;
    // Строка данных короче ожидаемого (битый файл) — пропускаем, чтобы не выйти за границы.
    if(index_time_ >= words.size() || words.size() < vec_canal.size()){
        return;
    }
    if(words[index_time_] == "-999.250"){
        return;
    }
    int index = words[index_time_].indexOf('.');
    QString word = words[index_time_].left(index);

    qint64 time = QDateTime::fromString(date_ +" "+ word, "dd/MM/yyyy hhmmss").toMSecsSinceEpoch();
    for(int i =1; i < vec_canal.size();++i){
        double num = words[i].toDouble();
        if(num == -999.25){
            continue;
        }
        if(vec_canal[i].first_unit){
            vec_canal[i].unit_max = num;
            vec_canal[i].unit_min = num;
            vec_canal[i].first_unit = false;
        }
        vec_canal[i].unit_max = qMax(vec_canal[i].unit_max, num);
        vec_canal[i].unit_min = qMin(vec_canal[i].unit_min, num);
        if(vec_canal[i].first_write_rectangle){
            QPointF point = vec_canal[i].points_rectangle.back();
            vec_canal[i].points_rectangle.push_back(QPointF(time,point.y()));
        }
        vec_canal[i].first_write_rectangle = true;
        vec_canal[i].points_rectangle.append(QPointF(time,num));
        vec_canal[i].points_triangle.append(QPointF(time,num));
    }
}
void Las::CreateSensor(){
    for (int i = 0 ;i < name_curve_.size();++i){
        if(name_curve_[i] == "TIME."){
            index_time_ = i;
        }
        CreateCanal(name_curve_[i]);
    }
}
void Las::CreateCanal(QString name){
    Canal acm;
    int index = name.indexOf('.');
    acm.axis_y_ = new QValueAxis();
    acm.select_box = new QCheckBox(name);
    acm.axis_y_->setTitleText(name);
    acm.axis_y_->setTickCount(21);
    acm.label_data = new QLabel();
    acm.first_name_canal = name;
    acm.name_canal = name.left(index);
    acm.name_unit = name.right(index);
    QPen pen;
    pen.setWidth(1);
    acm.series = new QLineSeries();
    acm.series->setPen(pen);
    acm.series->setName(name);
    acm.label = new QLabel();
    acm.label_delta = new QLabel();
    acm.label_data = new QLabel();
    acm.label_name_canal = new QLabel(acm.name_canal);
    //acm.name_sensor = name_sensor;
    acm.label_name_sensor = new QLabel();
    data_.vec_canal.push_back(acm);
}
void Las::SetNameSensor(const QStringList& words){
    if(words.size() < 2){
        return;
    }
    int index = words[1].indexOf(':');
    data_.name_sensor = words[1].left(index);
}
void Las::SetNumSensor(const QStringList& words){
    if(words.size() < 2){
        return;
    }
    int index = words[1].indexOf(':');
    data_.number_sensor = words[1].left(index);
}
