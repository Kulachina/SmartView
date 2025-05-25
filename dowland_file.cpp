#include "dowland_file.h"
#include <QVBoxLayout>
#include <QtMath>

DowlandFile::DowlandFile(DataBase& data_base)
    : data_base_(data_base)
{
    w_progress_ = new QWidget();
    w_progress_->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    QVBoxLayout *vb = new QVBoxLayout();
    progress_ = new QProgressBar();
    //progress_->setTextVisible(false);
    vb->addWidget(progress_);
    w_progress_->setLayout(vb);

}
void DowlandFile::LoadDocACM(QString path){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){
        qWarning() << "Не удалось открыть файл для чтения:" << path;
        return;
    }
    QTextStream in(&file);
    QString all_text = in.readAll();
    file.close();
    QStringList line_text = all_text.split("\n",Qt::SkipEmptyParts);
    QStringList words;
    int size = line_text.size();
    progress_->setRange(0,size);
    progress_->setValue(0);
    w_progress_->show();
    words = line_text[7].split(" ",Qt::SkipEmptyParts);
    CreateSeriesACM(words);
    int i = line_text.size()-2;
    words = line_text[i].split(" ",Qt::SkipEmptyParts);
    axis_x_->setMax(QDateTime::fromMSecsSinceEpoch(TextToInt(words[0]+ " " +words[1])));
    for(int i = 9;i < line_text.size()-1; ++ i){
        progress_->setValue(i);
        QCoreApplication::processEvents();
        words = line_text[i].split(" ",Qt::SkipEmptyParts);
        AddDataACM(words);
    }
    w_progress_->hide();
    DataSeriesACM& data = data_acm_.back();
    data.series_bar->replace(p_bar_);
    data.series_temp->replace(p_temp_);
    first_min_max_ = false;
    p_temp_.clear();
    p_bar_.clear();
    axis_bar_->setRange(bar_min_,bar_max_ + bar_max_ * 0.1);
    axis_temp_->setRange(temp_min_,temp_max_ + temp_max_ * 0.1);
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
            CreateSeriesEtalon(header_words);
        }
    }
    int i = 0;
    progress_->setRange(0,14000);
    progress_->setValue(0);
    w_progress_->show();
    DataEtalon data;
    char data_magic[4];
    while (!in.atEnd()) {
        progress_->setValue(i);
        QCoreApplication::processEvents();
        ++i;
        if (in.readRawData(data_magic, sizeof(data_magic)) == sizeof(data_magic) && strncmp(data_magic, "DATA", 4) == 0) {
            in >> data.time >> data.value_1 >> data.value_2 >> data.gap_series_1 >> data.gap_series_2 >> data.check_point;
        }
        AddDataEtalon(data); 
    }
    axis_x_->setMax(QDateTime::fromMSecsSinceEpoch(data.time));
    w_progress_->hide();
    data_etalon_[0].series->replace(p_temp_);
    data_etalon_[1].series->replace(p_bar_);
    p_temp_.clear();
    p_bar_.clear();
    data_etalon_[1].axis_y_->setRange(bar_min_,bar_max_ + bar_max_ * 0.1);
    data_etalon_[0].axis_y_->setRange(temp_min_,temp_max_ + temp_max_ * 0.1);
    file.close();
}
void DowlandFile::CreateSeriesACM(QStringList words){
    DataSeriesACM data;
    data.name_sensor = words[2] + " " + words[3].trimmed();
    data.label_sensor = new QLabel(data.name_sensor);
    data.data_sensor_bar = new QLabel();
    data.data_sensor_bar->setFixedWidth(50);
    data.data_sensor_temp = new QLabel();
    data.data_sensor_temp->setFixedWidth(50);
    data.series_bar = new QLineSeries();
    data.series_bar->setName(data.name_sensor + "bar");
    data.series_bar->setColor("red");
    data.series_temp = new QLineSeries();
    data.series_temp->setName(data.name_sensor + "temp");
    data.series_temp->setColor("blue");
    chart_->addSeries(data.series_temp);
    chart_->addSeries(data.series_bar);
    data.series_bar->attachAxis(axis_bar_);
    data.series_temp->attachAxis(axis_temp_);
    data.series_bar->attachAxis(axis_x_);
    data.series_temp->attachAxis(axis_x_);
    data_acm_.push_back(data);
    data_base_.AddDataSerACM(data_acm_.back());
}
void DowlandFile::CreateSeriesEtalon(QStringList words){
    DataSeriesEtalon doc;
    for(int i =1 ;i <= words.size()-1;++i){
        doc.name_series = words[i];
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
        doc.series->setName(words[i]);
        chart_->addSeries(doc.series);
        chart_->addSeries(doc.point_series);
        chart_->addAxis(doc.axis_y_,Qt::AlignLeft);
        if(words[i] == "ЛТ300" || words[i] == "Имитатор ЛТ300"){
            doc.axis_y_->setTitleText("Эталон Температура, °C");
            doc.series->setColor("blue");
            doc.series->attachAxis(doc.axis_y_);
            doc.point_series->attachAxis(doc.axis_y_);
        }
        if(words[i] == "ДМ5002М" || words[i] == "Имитатор ДМ5002М"){
            doc.axis_y_->setTitleText("Эталон Давление, кг/см2");
            doc.series->setColor("red");
            doc.series->attachAxis(doc.axis_y_);
            doc.point_series->attachAxis(doc.axis_y_);
        }
        doc.name_series = words[i];
        doc.point_series->attachAxis(axis_x_);
        doc.series->attachAxis(axis_x_);
        data_etalon_.push_back(doc);
        data_base_.AddDataSerEtalon(data_etalon_.back());
        data_base_.AddListAxis(data_etalon_.back().axis_y_);
    }
}
void DowlandFile::AddDataACM(QStringList words){
    double temp = words[3].toDouble();
    double bar =words[2].toDouble();
    if(!first_min_max_){
        axis_x_->setMin(QDateTime::fromMSecsSinceEpoch(TextToInt(words[0]+ " " +words[1])));
        bar_max_ = bar;
        bar_min_ = bar;
        temp_max_ = temp;
        temp_min_ = temp;
        first_min_max_ = true;
    }
    SetMinMaxY(temp,bar);
    qint64 time = TextToInt(words[0]+ " " + words[1]);
    p_bar_.append(QPointF(time,bar));
    p_temp_.append(QPointF(time,temp));
}

void DowlandFile::AddDataEtalon(DataEtalon data){
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
        error_flag_1_ = true;
    }
    if(data_etalon_[1].series && (data.value_2 != 999)){
        p_bar_.append(QPointF(data.time,data.value_2));
        error_flag_2_ = true;
    }
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
    doc.old_series.push_back(doc.series);
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
    bar_min_ = qMin(bar_min_,bar);
    bar_max_ = qMax(bar_max_,bar);
    temp_max_ = qMax(temp_max_,temp);
    temp_min_ = qMin(temp_min_,temp);

    /*if(bar_min_ > bar){
        bar_min_ = bar;
    }
    if(bar_max_ < bar){
        bar_max_ = bar;
    }
    if(temp_min_> temp){
        temp_min_ = temp;
    }
    if(temp_max_ < temp){
        temp_max_ = temp;
    }*/
}
