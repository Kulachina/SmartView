#include "dowland_file.h"
#include <QVBoxLayout>
#include <QtMath>
#include <QPushButton>

DowlandFile::DowlandFile(DataBase& data_base)
    : data_base_(data_base)
{
    w_select_chart_ = new QWidget();
    prog_ = new QProgressBar();
    prog_->setWindowFlags(Qt::Window | Qt::WindowTitleHint);
    prog_->setWindowTitle("Загрузка");

}
DataSeriesSensor DowlandFile::LoadDocAMT(QString path, QString name, int count_file, int count_now){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){
        qWarning() << "Не удалось открыть файл для чтения:" << path;
        return DataSeriesSensor{};
    }
    QTextStream in(&file);
    QString all_text = in.readAll();
    file.close();
    QStringList line_text = all_text.split("\n",Qt::SkipEmptyParts);
    QStringList words;
    int size = line_text.size();
    prog_->setWindowTitle("Загрузка " + QString::number(count_now) + " из " + QString::number(count_file));
    prog_->setRange(0,size);
    prog_->setValue(0);
    prog_->show();
    chart_name_and_index_.push_back({"Давление","кгс/см2",name});
    chart_name_and_index_.push_back({"Температура","С",name});
    index_data_ = 2;
    DataSeriesSensor data;
    CreateSeriesACM(data);
    int i = line_text.size()-2;
    words = line_text[i].split("\t",Qt::SkipEmptyParts);
    qint64 time = TextToInt(words[0].trimmed()+ " " +words[1].trimmed());
    axis_x_->setMax(QDateTime::fromMSecsSinceEpoch(time));
    for(int i = index_data_;i < line_text.size()-1; ++i){
        prog_->setValue(i);
        QCoreApplication::processEvents();
        words = line_text[i].split("\t",Qt::SkipEmptyParts);
        AddDataACM(words, data);
    }
    prog_->hide();
    QVector<Canal>& acm = data.vec_canal;
    for(int i =0; i < acm.size();++i){
        acm[i].series->replace(acm[i].points_rectangle);
        if(acm[i].name_canal.contains("Давление",Qt::CaseInsensitive)){
            axis_bar_->setRange(acm[i].unit_min,acm[i].unit_max +acm[i].unit_max * 0.1);
        }
        if(acm[i].name_canal.contains("Температура",Qt::CaseInsensitive)){
            axis_temp_->setRange(acm[i].unit_min,acm[i].unit_max +acm[i].unit_max * 0.1);
        }
    }
    chart_name_and_index_.clear();
    return data;
}
void DowlandFile::LoadTXTEtalon(QString path){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){
        qWarning() << "Не удалось открыть файл для чтения:" << path;
        return ;
    }
    QTextStream in(&file);
    QString all_text = in.readAll();
    file.close();
    QStringList line_text = all_text.split("\n",Qt::SkipEmptyParts);
    QStringList words;
    int size = line_text.size();
    prog_->setWindowTitle("Загрузка Эталона");
    prog_->setRange(0,size);
    prog_->setValue(0);
    prog_->show();
    DataSeriesEtalon data;
    QStringList zagolovok = line_text[0].split("\t",Qt::SkipEmptyParts);
    QStringList min_axis = line_text[1].split("\t",Qt::SkipEmptyParts);
    data_etalon_.reserve(2);
    QList<QPointF> points_bar;
    QList<QPointF> points_temp;
    QList<QPointF> points_rect_bar;
    QList<QPointF> points_rect_temp;
    int index = line_text.size()-5;
    words = line_text[index].split("\t",Qt::SkipEmptyParts);
    axis_x_->setMax(QDateTime::fromMSecsSinceEpoch(TextToInt(words[0].trimmed()+ " " +words[1].trimmed())));
    axis_x_->setMin(QDateTime::fromMSecsSinceEpoch(TextToInt(min_axis[0].trimmed()+ " " +min_axis[1].trimmed())));
    qint64 time;
    double temp;
    double bar;
    bool first_write_rect = false;
    bool ok_bar;
    bool ok_temp;
    double max_y_temp = 0;
    double max_y_bar = 0 ;
    for(int i = 1;i < index; ++i){
        prog_->setValue(i);
        QCoreApplication::processEvents();
        words = line_text[i].split("\t",Qt::SkipEmptyParts);
        time = TextToIntEtalon(words[0].trimmed()+ " " + words[1].trimmed());
        if(zagolovok[1] == "ЛТ300"){
            temp = words[2].replace(',','.').toDouble(&ok_temp);
            bar = words[3].replace(',','.').toDouble(&ok_bar);
        } else {
            temp = words[3].replace(',','.').toDouble(&ok_temp);
            bar = words[2].replace(',','.').toDouble(&ok_bar);
        }
        if(!ok_temp){
            temp = 0;
        }
        if(!ok_bar){
            bar = 0;
        }
        if(max_y_bar < bar){
            max_y_bar = bar;
        }
        if(max_y_temp < temp){
            max_y_temp = temp;
        }
        points_temp.push_back(QPointF(time,temp));
        points_bar.push_back(QPointF(time,bar));
        if(first_write_rect){
            QPointF point_b = points_bar.back();
            QPointF point_t = points_temp.back();
            points_rect_bar.push_back(QPointF(time,point_b.y()));
            points_rect_temp.push_back(QPointF(time,point_t.y()));
        }
        first_write_rect = true;
        points_rect_bar.push_back(QPointF(time,bar));
        points_rect_temp.push_back(QPointF(time,temp));
    }
    prog_->hide();

    if(zagolovok[1] == "ЛТ300"){
        CreateSeriesEtalon(zagolovok[1]);
        CreateSeriesEtalon(zagolovok[2]);
        data_etalon_[0].series->replace(points_rect_temp);
        data_etalon_[0].points_rectangle_view = points_rect_temp;
        data_etalon_[0].points_triangle_view = points_temp;
        data_etalon_[0].axis_y_->setRange(0,max_y_temp);
        data_etalon_[1].series->replace(points_rect_bar);
        data_etalon_[1].points_rectangle_view = points_rect_bar;
        data_etalon_[1].points_triangle_view = points_bar;
        data_etalon_[1].axis_y_->setRange(0,max_y_bar);
        data_base_.GetDataSerEtalon().push_back(data_etalon_[0]);
        data_base_.GetDataSerEtalon().push_back(data_etalon_[1]);
        data_base_.AddListAxis(data_etalon_[0].axis_y_);
        data_base_.AddListAxis(data_etalon_[1].axis_y_);
    } else{
        CreateSeriesEtalon(zagolovok[2]);
        CreateSeriesEtalon(zagolovok[1]);
        data_etalon_[1].series->replace(points_rect_temp);
        data_etalon_[1].points_rectangle_view = points_rect_temp;
        data_etalon_[1].points_triangle_view = points_temp;
        data_etalon_[1].axis_y_->setRange(0,max_y_bar);
        data_etalon_[0].series->replace(points_rect_bar);
        data_etalon_[0].points_rectangle_view = points_rect_bar;
        data_etalon_[0].points_triangle_view = points_bar;
        data_etalon_[0].axis_y_->setRange(0,max_y_temp);
        data_base_.GetDataSerEtalon().push_back(data_etalon_[1]);
        data_base_.GetDataSerEtalon().push_back(data_etalon_[0]);
        data_base_.AddListAxis(data_etalon_[1].axis_y_);
        data_base_.AddListAxis(data_etalon_[0].axis_y_);
    }
     axis_x_->setRange(QDateTime::fromMSecsSinceEpoch(points_bar.front().x()),QDateTime::fromMSecsSinceEpoch(points_bar.back().x()));
}

DataSeriesSensor DowlandFile::LoadDocACM(QString path, int count_file, int count_now){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){
        qWarning() << "Не удалось открыть файл для чтения:" << path;
        return DataSeriesSensor{};
    }
    QTextStream in(&file);
    QString all_text = in.readAll();
    file.close();
    QStringList line_text = all_text.split("\n",Qt::SkipEmptyParts);
    QStringList words;
    int size = line_text.size();
    bool check = SelectChart(line_text);
    if(!check){
        chart_name_and_index_.clear();
        return {};
    }
    prog_->setWindowTitle("Загрузка " + QString::number(count_now) + " из " + QString::number(count_file));
    prog_->setRange(0,size);
    prog_->setValue(0);
    prog_->show();
    DataSeriesSensor data;
    CreateSeriesACM(data);
    int index = line_text.size()-2;
    words = line_text[index].split(" ",Qt::SkipEmptyParts);
    axis_x_->setMax(QDateTime::fromMSecsSinceEpoch(TextToInt(words[0].trimmed()+ " " +words[1].trimmed())));
    for(int i = index_data_;i < line_text.size()-1; ++i){
        prog_->setValue(i);
        QCoreApplication::processEvents();
        words = line_text[i].split(" ",Qt::SkipEmptyParts);
        AddDataACM(words, data);
    }
    prog_->hide();
    QVector<Canal>& acm = data.vec_canal;
    for(int i =0; i < acm.size();++i){
        acm[i].series->replace(acm[i].points_rectangle);
        if(acm[i].name_canal.contains("Давление",Qt::CaseInsensitive)){
            axis_bar_->setRange(acm[i].unit_min,acm[i].unit_max +acm[i].unit_max * 0.1);
        }
        if(acm[i].name_canal.contains("Температура",Qt::CaseInsensitive)){
           axis_temp_->setRange(acm[i].unit_min,acm[i].unit_max +acm[i].unit_max * 0.1);
        }
    }
    chart_name_and_index_.clear();
    return data;
}
bool DowlandFile::SelectChart(QStringList& words){
    QStringList word;
    for(int i = 0 ; i < words.size()-1; ++i){
        if(words[i].size() < 2){
            index_data_ = i +1;
            return true;
        }
        word = words[i].split(" ",Qt::SkipEmptyParts);
        if(word.size() < 5){
            return false;
        }
        chart_name_and_index_.push_back({word[0]+ " " + word[1],word[2],word[4] +" "+ word[5]});
    }
    return true;
}
void DowlandFile::LoadDocEtalon_2v(QString path){
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть файл для чтения:" << path;
        return;
    }
    QDataStream in(&file);
    QString name;
    QList<QPointF> points;
    QVector<QPointF> points_rect;
    QVector<QPointF> check_points;
    double min;
    double max;
    data_etalon_.reserve(2);
    for(int i = 0; i < 2;++i){
        in >> name;
        CreateSeriesEtalon(name);
        in >> points;
        in >> points_rect;
        in >> min;
        in >> max;
        in >> check_points;
        CreateVecCheckPoints(name, check_points);
        data_etalon_.back().series->replace(points_rect);
        data_etalon_.back().points_rectangle_view = points_rect;
        data_etalon_.back().points_triangle_view = points;
        data_etalon_.back().point_series->replace(check_points);
        data_etalon_.back().axis_y_->setRange(min,max + max * 0.1);
    }
    if(data_etalon_[0].name_series == "ДМ5002М"){
        data_base_.GetDataSerEtalon().push_back(data_etalon_[1]);
        data_base_.GetDataSerEtalon().push_back(data_etalon_[0]);
        data_base_.AddListAxis(data_etalon_[1].axis_y_);
        data_base_.AddListAxis(data_etalon_[0].axis_y_);
    } else {
        data_base_.AddListAxis(data_etalon_[0].axis_y_);
        data_base_.AddListAxis(data_etalon_[1].axis_y_);
        data_base_.GetDataSerEtalon().push_back(data_etalon_[0]);
        data_base_.GetDataSerEtalon().push_back(data_etalon_[1]);
    }
    axis_x_->setRange(QDateTime::fromMSecsSinceEpoch(points.front().x()),QDateTime::fromMSecsSinceEpoch(points.back().x()));
    data_base_.SetDefaultAxisX(axis_x_->max(),axis_x_->min());
    for(QPointF& p : check_points){
        QDateTime time = QDateTime::fromMSecsSinceEpoch(p.x());
        data_base_.GetCheckPoints().push_back(time);
    }
}
void DowlandFile::CreateVecCheckPoints(QString name, QVector<QPointF> list){
    QVector<double> vec;
    for(QPointF p : list){
        vec.push_back(p.y());
    }
    if(name == "ЛТ300"|| name == "Имитатор ЛТ300"){
        data_base_.GetCheckPointTemp() = vec;
    }
    if(name == "ДМ5002М" || name == "Имитатор ДМ5002М"){
        data_base_.GetCheckPointBar() = vec;
    }
}
void DowlandFile::LoadDocEtalon(QString path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть файл для чтения:" << path;
        return;
    }
    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);
    QStringList header_words;
    DataHeaderEtalon header;
    char headerMagic[4];
    if (in.readRawData(headerMagic, sizeof(headerMagic)) == sizeof(headerMagic)) {
        if (strncmp(headerMagic, "HEAD", 4) == 0) {
            in >> header.header_1 >> header.header_2;
            header_words << "Время" << header.header_1 << header.header_2;
            for(int i =1 ;i <= header_words.size()-1;++i)
            CreateSeriesEtalon(header_words[i]);
        }
    }
    int i = 0;
    prog_->setRange(0,14000);
    prog_->setValue(0);
    prog_->show();
    DataEtalon data;
    char data_magic[4];
    while (!in.atEnd()) {
        prog_->setValue(i);
        QCoreApplication::processEvents();
        ++i;
        if (in.readRawData(data_magic, sizeof(data_magic)) == sizeof(data_magic) && strncmp(data_magic, "DATA", 4) == 0) {
            in >> data.time >> data.value_1 >> data.value_2 >> data.gap_series_1 >> data.gap_series_2 >> data.check_point;
        }
        AddDataEtalon(data); 
    }
    axis_x_->setMax(QDateTime::fromMSecsSinceEpoch(data.time));
    prog_->hide();
    data_etalon_[1].axis_y_->setRange(bar_min_,bar_max_ + bar_max_ * 0.1);
    data_etalon_[0].axis_y_->setRange(temp_min_,temp_max_ + temp_max_ * 0.1);
    data_base_.SetDefaultAxisX(axis_x_->max(),axis_x_->min());
    data_base_.GetDataSerEtalon().push_back(data_etalon_[0]);
    data_base_.GetDataSerEtalon().push_back(data_etalon_[1]);
    data_base_.GetDataSerEtalon()[1].points_triangle_view = p_bar_;
    data_base_.GetDataSerEtalon()[0].points_triangle_view = p_temp_;
    data_base_.GetDataSerEtalon()[0].series->replace(data_base_.GetDataSerEtalon()[0].points_rectangle_view);
    data_base_.GetDataSerEtalon()[1].series->replace(data_base_.GetDataSerEtalon()[1].points_rectangle_view);
    p_temp_.clear();
    p_bar_.clear();
    set_axis_x_ = false;
    error_flag_1_ = true;
    error_flag_2_ = true;
    create_file_ = false;
    create_title_ = false;
    first_write_rectangle_ =false;
    gap_ = false;
    first_min_max_ = false;
    file.close();
}
void DowlandFile::CreateSeriesACM(DataSeriesSensor& data){
    data.name_sensor = chart_name_and_index_[0].name_sensor;
    data.label_sensor = new QLabel(data.name_sensor);
    for(NameChart name_canal : chart_name_and_index_){
        CreateACM(data,name_canal.name_canal,name_canal.name_sensor,name_canal.name_unit);
    }
}
void DowlandFile::CreateACM(DataSeriesSensor& data, QString name_canal, QString name_sensor, QString name_unit){
    Canal acm;
    acm.label_data = new QLabel();
    acm.first_name_canal = name_canal;
    acm.name_canal = name_canal;
    acm.label_name_canal = new QLabel(name_canal);
    acm.name_unit = name_unit;
    acm.name_sensor = name_sensor;
    acm.label_name_sensor = new QLabel(name_sensor);
    acm.select_box = false;
    QPen pen;
    pen.setWidth(1);
    acm.axis_y_ = new QValueAxis();
    //acm.axis_y_->setTitleText(data.name_sensor + " " + name_canal);
    acm.axis_y_->setTickCount(21);
    acm.series = new QLineSeries();
    acm.series->setPen(pen);
    acm.series->setName(data.name_sensor + name_canal);
    acm.label = new QLabel();
    acm.label_delta = new QLabel();
    data.vec_canal.push_back(acm);
}
void DowlandFile::CreateSeriesEtalon(QString word){
    DataSeriesEtalon doc;
    doc.name_series = word;
    doc.series = new QLineSeries();
    doc.data_sensor = new QLabel();
    doc.axis_y_ = new QValueAxis();
    doc.axis_y_->setRange(0, 0);
    doc.axis_y_->setTickCount(21);
    doc.data_sensor->setFixedWidth(50);
    doc.point_series = new QScatterSeries();
    doc.point_series->setName("check_series");
    doc.point_series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    doc.point_series->setPointLabelsVisible(true);
    doc.point_series->setPointLabelsFormat("@yPoint");
    doc.point_series->setMarkerSize(5);
    doc.point_series->setColor("green");
    doc.label_point = new QLabel();
    doc.series->setName(word);
    chart_->addSeries(doc.series);
    chart_->addSeries(doc.point_series);
    chart_->addAxis(doc.axis_y_,Qt::AlignLeft);
    if(word == "ЛТ300" || word == "Имитатор ЛТ300"){
        doc.axis_y_->setTitleText("Эталон Температура, °C");
        doc.series->setColor("blue");
        QLineSeries* first_line = qobject_cast<QLineSeries*>(chart_->series().value(0));
        first_line->attachAxis(doc.axis_y_);
        doc.series->attachAxis(doc.axis_y_);
        doc.point_series->attachAxis(doc.axis_y_);
    }
    if(word == "ДМ5002М" || word == "Имитатор ДМ5002М"){
        doc.axis_y_->setTitleText("Эталон Давление, кг/см2");
        doc.series->setColor("red");
        doc.series->attachAxis(doc.axis_y_);
        doc.point_series->attachAxis(doc.axis_y_);
    }
    doc.name_series = word;
    doc.point_series->attachAxis(axis_x_);
    doc.series->attachAxis(axis_x_);
    data_etalon_.push_back(doc);
}
void DowlandFile::AddDataACM(QStringList words, DataSeriesSensor& data){
    QVector<Canal>& vec_canal = data.vec_canal;
    qint64 time = TextToInt(words[0].trimmed()+ " " + words[1].trimmed());
    for(int i =0; i < vec_canal.size();++i){
        words[i+2].replace(',','.');
        double num = words[i+2].toDouble();
        if(num < -100){
            num = 0;
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

void DowlandFile::AddDataEtalon(DataEtalon data){
    QVector<QPointF>& points_bar = data_etalon_[1].points_rectangle_view;
    QVector<QPointF>& points_temp = data_etalon_[0].points_rectangle_view;
    if(first_write_rectangle_){
        QPointF point_bar = points_bar.back();
        QPointF point_temp = points_temp.back();
        points_bar.push_back(QPointF(data.time,point_bar.y()));
        points_temp.push_back(QPointF(data.time,point_temp.y()));
    }
    first_write_rectangle_ = true;
    if(!set_axis_x_){
        axis_x_->setMin(QDateTime::fromMSecsSinceEpoch(data.time));
        bar_max_ = data.value_2;
        bar_min_ = data.value_2;
        temp_max_ = data.value_1;
        temp_min_ = data.value_1;
        set_axis_x_ = true;
    }
    SetMinMaxY(data.value_1,data.value_2);
    if(gap_){
        for(DataSeriesEtalon& doc : data_etalon_){
            GapSeries(doc);
        }
        gap_ = false;
    }
    if(data.gap_series_1 && data.gap_series_2){
        gap_ = true;
    }
    if(data.check_point){
        data_base_.AddCheckPoint(data.time,data.value_1,data.value_2);
        check_points_bar_.append(QPointF(data.time,data.value_2));
        check_points_temp_.append(QPointF(data.time,data.value_1));
        data_etalon_[0].point_series->append(data.time,data.value_1);
        data_etalon_[1].point_series->append(data.time,data.value_2);
    }
    if((data.value_1 == 999) && error_flag_1_){
        DataSeriesEtalon& doc = data_etalon_[0];
        data_etalon_[0].series->replace(p_temp_);
        p_temp_.clear();
        GapSeries(doc);
        error_flag_1_ = false;
    }
    if((data.value_2 == 999) && error_flag_2_){
        DataSeriesEtalon& doc = data_etalon_[1];
        data_etalon_[1].series->replace(p_bar_);
        p_bar_.clear();
        GapSeries(doc);
        error_flag_2_ = false;
    }
    if(data_etalon_[0].series && (data.value_1 != 999)){
        p_temp_.append(QPointF(data.time,data.value_1));   
        points_temp.push_back(QPointF(data.time,data.value_1));
        error_flag_1_ = true;
    }
    if(data_etalon_[1].series && (data.value_2 != 999)){
        p_bar_.append(QPointF(data.time,data.value_2));
        points_bar.push_back(QPointF(data.time,data.value_2));
        error_flag_2_ = true;
    }
}
qint64 DowlandFile::TextToIntEtalon(QString word){
    qint64 time = QDateTime::fromString(word, "yyyy.MM.dd hh:mm:ss").toMSecsSinceEpoch();
    return time;
}
qint64 DowlandFile::TextToInt(QString word){
    qint64 time = QDateTime::fromString(word, "dd.MM.yyyy hh:mm:ss").toMSecsSinceEpoch();
    return time;
}
void DowlandFile::SetAxisTime(QDateTimeAxis *axis_x){
    axis_x_ = axis_x;
}
QDateTime DowlandFile::GetAxisTime(){
    return now_time_;
}
void DowlandFile::SetChartDoc(QChart* chart,QValueAxis* axis_temp,QValueAxis* axis_bar){
    chart_ = chart;
    axis_temp_ = axis_temp;
    axis_bar_ = axis_bar;
}
QVector<DataSeriesEtalon>& DowlandFile::GetDataSeriesEtalon(){
    if(data_etalon_.empty()){
    }
    return data_etalon_;
}
void DowlandFile::GapSeries(DataSeriesEtalon& doc){
    QLineSeries *old_s = doc.series;
    doc.old_series.push_back(old_s);
    doc.series = new QLineSeries();
    chart_->addSeries(doc.series);
    if(doc.name_series == "ЛТ300" || doc.name_series == "Имитатор ЛТ300"){
        doc.series->setColor("blue");
    }
    if(doc.name_series == "ДМ5002М" || doc.name_series == "Имитатор ДМ5002М"){
        doc.series->setColor("red");  
    }
    doc.series->attachAxis(doc.axis_y_);
    doc.series->attachAxis(axis_x_);
    doc.series->setName(doc.name_series);
    doc.series->setPointLabelsFormat("@yPoint");
    doc.series->setPointLabelsClipping(false);
}
void DowlandFile::CheckFlag(){
    create_file_ = false;
    create_title_ = false;
}
void DowlandFile::SetMinMaxY(double temp, double bar){
    double b = 0;
    double t = 0;
    if(bar < -50 ){
        b = 0;
    } else {
        b = bar;
    }
    if (temp < -50){
        t = 0;
    } else {
        t = temp;
    }
    bar_min_ = qMin(bar_min_,b);
    bar_max_ = qMax(bar_max_,b);
    temp_max_ = qMax(temp_max_,t);
    temp_min_ = qMin(temp_min_,t);
}
void DowlandFile::ClearAll(){
    data_etalon_.clear();
    data_acm_.clear();
    p_bar_.clear();
    p_temp_.clear();
    bar_max_ = 0;
    bar_min_ = 0;
    temp_max_ = 0;
    temp_min_ = 0;
    set_axis_x_ = false;
    error_flag_1_ = true;
    error_flag_2_ = true;
    create_file_ = false;
    create_title_ = false;
    gap_ = false;
    first_min_max_ = false;
}
void DowlandFile::LoadSVDoc(const QString path){
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть файл для чтения:" << path;
        return;
    }
    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);
    LoadDataEt(in);
    LoadDataACM(in);

    file.close();
}


void DowlandFile::LoadDataEt(QDataStream& in){
    quint32 size;
    in >> size;
    DataSeriesEtalon data;
    for(quint32 i = 0;i < size; ++i){
        in >> data.name_series;
        CreateSeriesEtalon(data.name_series);
        in >> data_base_.GetDataSerEtalon()[i].points_triangle_view
            >> data_base_.GetDataSerEtalon()[i].points_rectangle_view;
    }
    qint64 min ;
    qint64 max ;
    double bar_max = 0,
        bar_min = 0,
        temp_max = 0,
        temp_min = 0;
    in >> bar_min
        >> bar_max
        >> temp_min
        >> temp_max
        >> min
        >> max
        >> check_points_bar_
        >> check_points_temp_
        >> data_base_.GetCheckPointTemp()
        >> data_base_.GetCheckPointBar()
        >> data_base_.GetCheckPoints64();
    data_base_.CreatePointsDate();
    axis_x_->setRange(QDateTime::fromMSecsSinceEpoch(min),
                      QDateTime::fromMSecsSinceEpoch(max));
    data_base_.GetDataSerEtalon()[1].axis_y_->setRange(bar_min,bar_max + bar_max * 0.1);
    data_base_.GetDataSerEtalon()[0].axis_y_->setRange(temp_min,temp_max + temp_max * 0.1);
    data_base_.GetDataSerEtalon()[0].point_series->replace(check_points_temp_);
    data_base_.GetDataSerEtalon()[1].point_series->replace(check_points_bar_);
    data_base_.GetDataSerEtalon()[1].series->replace(data_base_.GetDataSerEtalon()[1].points_rectangle_view);
    data_base_.GetDataSerEtalon()[0].series->replace(data_base_.GetDataSerEtalon()[0].points_rectangle_view);
}

void DowlandFile::LoadDataACM(QDataStream& in){
    quint32 size;
    in >> size;
    for(quint32 i = 0;i < size;++i){
        DataSeriesSensor data;
        in >> data.name_sensor;
        data.label_sensor = new QLabel(data.name_sensor);
        quint32 size_acm;
        in >> size_acm;
        data.vec_canal.resize(size_acm);
        for(quint32 j = 0;j < size_acm;++j){
            Canal& acm = data.vec_canal[j];
            in >> acm.name_canal
                >> acm.name_canal
                >> acm.name_unit
                >> acm.unit_max
                >> acm.unit_min
                >> acm.check_points
                >> acm.delta_points
                >> acm.points_rectangle
                >> acm.points_triangle;
            acm.label_data = new QLabel();
            acm.label_name_canal = new QLabel(acm.name_canal);
            acm.label_name_sensor = new QLabel(data.name_sensor);
            QPen pen;
            pen.setWidth(1);
            acm.series = new QLineSeries();
            acm.series->setPen(pen);
            acm.series->setName(data.name_sensor + acm.name_canal);
            chart_->addSeries(acm.series);
            acm.series->replace(acm.points_rectangle);
            if(acm.name_canal.contains("Давление",Qt::CaseInsensitive)){
                acm.series->attachAxis(axis_bar_);
                acm.series->setColor("red");
                acm.series->setObjectName("bar");
            }
            if(acm.name_canal.contains("Температура",Qt::CaseInsensitive)){
                acm.series->attachAxis(axis_temp_);
                acm.series->setColor("blue");
                acm.series->setObjectName("temp");
            }
            acm.label = new QLabel();
            acm.label_delta = new QLabel();
            acm.series->attachAxis(axis_x_);
            if(acm.name_canal.contains("Давление",Qt::CaseInsensitive)){
                axis_bar_->setRange(acm.unit_min,acm.unit_max +acm.unit_max * 0.1);
            }
            if(acm.name_canal.contains("Температура",Qt::CaseInsensitive)){
                axis_temp_->setRange(acm.unit_min,acm.unit_max +acm.unit_max * 0.1);
            }
        }
        data_acm_.push_back(data);
        data_base_.AddDataSerACM(data_acm_.back());
    }
}

void DowlandFile::SaveSVDoc(const QString path){
    if (path.isEmpty()) {
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(nullptr, "Ошибка", "Не удалось открыть файл для записи");
        return;
    }
    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);
    SaveDataEt(out);
    SaveDataACM(out);
    file.close();
}
void DowlandFile::SaveDataEt(QDataStream& out){
    out << static_cast<quint32>(data_base_.GetDataSerEtalon().size());
    for(DataSeriesEtalon& data : data_base_.GetDataSerEtalon()){
        out << data.name_series;
           out << data.points_triangle_view
            << data.points_rectangle_view;
    }
    qint64 min = axis_x_->min().toMSecsSinceEpoch();
    qint64 max = axis_x_->max().toMSecsSinceEpoch();
    out << bar_min_
        << bar_max_
        << temp_min_
        << temp_max_
        << min
        << max
        << check_points_bar_
        << check_points_temp_
        << data_base_.GetCheckPointTemp()
        << data_base_.GetCheckPointBar()
        << data_base_.GetCheckPoints64();
}
void DowlandFile::SaveDataACM(QDataStream& out){
    out << static_cast<quint32>(data_base_.GetDataSerACM().size());
    for(DataSeriesSensor& data : data_base_.GetDataSerACM()){
        out << data.name_sensor;
        out << static_cast<quint32>(data.vec_canal.size());
        for(Canal& acm : data.vec_canal){
            out << acm.name_canal
                << acm.name_canal
                << acm.name_unit
                << acm.unit_max
                << acm.unit_min
                << acm.check_points
                << acm.delta_points
                << acm.points_rectangle
                << acm.points_triangle;
        }
    }
}

