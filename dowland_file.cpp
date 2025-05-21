#include "dowland_file.h"
#include <QPointer>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDataStream>
#include <QStandardPaths>
#include <QElapsedTimer>

DowlandFile::DowlandFile(DataBase& data_base)
    : data_base_(data_base)
{

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
    for(int i = 0;i < line_text.size()-1; ++ i){
        if(i == 7){
            words = line_text[i].split(" ",Qt::SkipEmptyParts);
            CreateSeriesACM(words);
        }
        if(i > 8){
            words = line_text[i].split(" ",Qt::SkipEmptyParts);
            AddDataACM(words);
        }
        if(i == line_text.size()-2){
            words = line_text[i].split(" ",Qt::SkipEmptyParts);
            axis_x_->setMax(QDateTime::fromMSecsSinceEpoch(TextToInt(words[1])));
        }

    }
    DataSeriesACM& data = data_acm_.back();
    data.series_bar->replace(p_bar_);
    data.series_temp->replace(p_temp_);
    first_min_max_ = false;
    p_temp_.clear();
    p_bar_.clear();
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
    while (!in.atEnd()) {
        DataEtalon data;
        char data_magic[4];
        if (in.readRawData(data_magic, sizeof(data_magic)) == sizeof(data_magic) && strncmp(data_magic, "DATA", 4) == 0) {
            in >> data.time >> data.value_1 >> data.value_2 >> data.gap_series_1 >> data.gap_series_2 >> data.check_point;
        }
        AddDataEtalon(data);
        axis_x_->setMax(QDateTime::fromMSecsSinceEpoch(data.time));
    }

    file.close();
}
void DowlandFile::CreateSeriesACM(QStringList words){
    DataSeriesACM data;
    data.name_sensor = words[2] + " " + words[3].trimmed();
    data.label_sensor = new QLabel(data.name_sensor);
    data.data_sensor_bar = new QLabel(".........");
    data.data_sensor_temp = new QLabel("..........");
    data.series_bar = new QLineSeries();
    data.series_bar->setName(data.name_sensor + "bar");
    data.series_bar->setColor("red");
    data.series_temp = new QLineSeries();
    data.series_temp->setName(data.name_sensor + "temp");
    data.series_temp->setColor("blue");
    data.axis_y_temp = new QValueAxis();
    data.axis_y_temp->setTitleText("Температура, °C");
    data.axis_y_temp->setTickCount(21);
    data.axis_y_temp->setRange(0,100);
    data.axis_y_temp->setVisible(false);
    data.axis_y_bar = new QValueAxis();
    data.axis_y_bar->setTitleText("Давление, кг/см2");
    data.axis_y_bar->setTickCount(21);
    data.axis_y_bar->setRange(0,1000);
    data.axis_y_bar->setVisible(false);
    chart_->addAxis(data.axis_y_temp, Qt::AlignLeft);
    chart_->addAxis(data.axis_y_bar, Qt::AlignLeft);
    chart_->addSeries(data.series_temp);
    chart_->addSeries(data.series_bar);
    data.series_bar->attachAxis(data.axis_y_bar);
    data.series_temp->attachAxis(data.axis_y_temp);
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
        doc.axis_y = new QValueAxis();
        doc.point_series = new QScatterSeries();
        doc.point_series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        doc.point_series->setPointLabelsVisible(true);
        doc.point_series->setPointLabelsFormat("@yPoint");
        doc.point_series->setMarkerSize(5);
        doc.point_series->setColor("green");
        doc.axis_y->setTickCount(21);
        doc.label_point = new QLabel();
        doc.series->setName(words[i]);
        if(words[i] == "ЛТ300" || words[i] == "Имитатор ЛТ300"){
            doc.series->setColor("blue");
            doc.axis_y->setRange(1000, 0);
            doc.axis_y->setTitleText("Температура, °C");
            doc.axis_y->setVisible(false);
        }
        if(words[i] == "ДМ5002М" || words[i] == "Имитатор ДМ5002М"){
            doc.series->setColor("red");
            doc.axis_y->setRange(1000, 0);
            doc.axis_y->setTitleText("Давление, кг/см2");
            doc.axis_y->setVisible(false);
        }
        doc.axis_y->setTitleBrush(QBrush(QColor(doc.series->color().name())));
        chart_->addAxis(doc.axis_y,Qt::AlignLeft);
        chart_->addSeries(doc.series);
        chart_->addSeries(doc.point_series);
        doc.name_series = words[i];
        doc.point_series->attachAxis(axis_x_);
        doc.point_series->attachAxis(doc.axis_y);
        doc.series->attachAxis(axis_x_);
        doc.series->attachAxis(doc.axis_y);
        data_etalon_.push_back(doc);
        data_base_.AddDataSerEtalon(data_etalon_.back());
    }
}
void DowlandFile::AddDataACM(QStringList words){
    double temp = words[3].toDouble();
    double bar =words[2].toDouble();
    if(!first_min_max_){
        axis_x_->setMin(QDateTime::fromMSecsSinceEpoch(TextToInt(words[1])));
        first_min_max_ = true;
    }
    double time = TextToInt(words[1]);

    p_bar_.append(QPointF(time,bar));
    p_temp_.append(QPointF(time,temp));

}

void DowlandFile::AddDataEtalon(DataEtalon data){
    if(!set_axis_x_){
        axis_x_->setMin(QDateTime::fromMSecsSinceEpoch(data.time));
        set_axis_x_ = true;
    }
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
        GapSeries(doc);
        error_flag_1_ = false;
    }
    if((data.value_2 == 999) && error_flag_2_){
        DataSeriesEtalon& doc = data_etalon_[1];
        GapSeries(doc);
        error_flag_2_ = false;
    }
    if(data_etalon_[0].series && (data.value_1 != 999)){
        SetAxisY(data_etalon_[0].axis_y,data.value_1);
        data_etalon_[0].series->append(data.time,data.value_1);
        error_flag_1_ = true;
    }
    if(data_etalon_[1].series && (data.value_2 != 999)){
        SetAxisY(data_etalon_[1].axis_y,data.value_2);
        data_etalon_[1].series->append(data.time,data.value_2);
        error_flag_2_ = true;
    }
}
qint64 DowlandFile::TextToInt(QString word){
    QDateTime time = QDateTime::fromString(word, "hh:mm:ss");
    time.setDate(QDate::currentDate());
    qint64 result = time.toMSecsSinceEpoch();
    return result;
}
void DowlandFile::SetAxisTime(QDateTimeAxis* axis_x){
    axis_x_ = axis_x;
}
QDateTime DowlandFile::GetAxisTime(){
    return now_time_;
}
void DowlandFile::SetChartDoc(QChart* chart){
    chart_ = chart;
}
QVector<DataSeriesEtalon>& DowlandFile::GetDataSeriesEtalon(){
    if(data_etalon_.empty()){
    }
    return data_etalon_;
}
void DowlandFile::GapSeries(DataSeriesEtalon& doc){
    doc.old_series.push_back(doc.series);
    doc.series = new QLineSeries();
    if(doc.name_series == "ЛТ300" || doc.name_series == "Имитатор ЛТ300"){
        doc.series->setColor("blue");
    }
    if(doc.name_series == "ДМ5002М" || doc.name_series == "Имитатор ДМ5002М"){
        doc.series->setColor("red");
    }
    chart_->addSeries(doc.series);
    doc.series->attachAxis(axis_x_);
    doc.series->attachAxis(doc.axis_y);
    doc.series->setName(doc.name_series);
    doc.series->setPointLabelsFormat("@yPoint");
    doc.series->setPointLabelsClipping(false);
}
void DowlandFile::CheckFlag(){
    create_file_ = false;
    create_title_ = false;
}
void DowlandFile::SetAxisY(QValueAxis* axis, double y){
    if(axis->min() > y ){
        axis->setMin(y + (y * 0.1));
    }
    if(axis->max() < y ){
        axis->setMax(y + (y * 0.1));
    }
}
