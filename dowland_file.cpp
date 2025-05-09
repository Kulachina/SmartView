#include "dowland_file.h"
#include <QPointer>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDataStream>
#include <QStandardPaths>

DowlandFile::DowlandFile(DataBase& data_base)
    : data_base_(data_base)
{

}
void DowlandFile::LoadDocumentData(QString path) {
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
            CreateSeries(header_words);
        }
    }
    while (!in.atEnd()) {
        DataEtalon data;
        char data_magic[4];
        if (in.readRawData(data_magic, sizeof(data_magic)) == sizeof(data_magic) && strncmp(data_magic, "DATA", 4) == 0) {
            in >> data.time >> data.value_1 >> data.value_2 >> data.gap_series_1 >> data.gap_series_2 >> data.check_point;
        }
        AddDataSeries(data);
    }
    file.close();
}
void DowlandFile::CreateSeries(QStringList words){
    DataSeriesEtalon doc;
    for(int i =1 ;i <= words.size()-1;++i){
        doc.name_series = words[i];
        doc.series = new QLineSeries();
        doc.axis_y = new QValueAxis();
        doc.point_series = new QScatterSeries();
        doc.point_series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        doc.point_series->setPointLabelsVisible(true);
        doc.point_series->setPointLabelsFormat("@yPoint");
        doc.point_series->setMarkerSize(15);
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
        data_document_.push_back(doc);
        data_base_.AddDataSerEtalon(data_document_.back());
    }
}
void DowlandFile::AddDataSeries(DataEtalon data){
    if(!set_axis_x_){
        QDateTime now = QDateTime::fromMSecsSinceEpoch(data.time);
        now_time_ = now;
        axis_x_->setRange(now, now.addSecs(100));
        set_axis_x_ = true;
    }
    if(gap_){
        for(DataSeriesEtalon& doc : data_document_){
            GapSeries(doc);
        }
        gap_ = false;
    }
    if(data.gap_series_1 && data.gap_series_2){
        gap_ = true;
    }
    if(data.check_point){
        data_document_[0].point_series->append(data.time,data.value_1);
        data_document_[1].point_series->append(data.time,data.value_2);
    }
    if((data.value_1 == 999) && error_flag_1_){
        DataSeriesEtalon& doc = data_document_[0];
        GapSeries(doc);
        error_flag_1_ = false;
    }
    if((data.value_2 == 999) && error_flag_2_){
        DataSeriesEtalon& doc = data_document_[1];
        GapSeries(doc);
        error_flag_2_ = false;
    }
    if(data_document_[0].series && (data.value_1 != 999)){
        SetAxisY(data_document_[0],data.value_1);
        data_document_[0].series->append(data.time,data.value_1);
        error_flag_1_ = true;
    }
    if(data_document_[1].series && (data.value_2 != 999)){
        SetAxisY(data_document_[1],data.value_2);
        data_document_[1].series->append(data.time,data.value_2);
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
    if(data_document_.empty()){
    }
    return data_document_;
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
void DowlandFile::SetAxisY(DataSeriesEtalon& data, double y){
    if(data.axis_y->min() > y ){
        data.axis_y->setMin(y);
    }
    if(data.axis_y->max() < y ){
        data.axis_y->setMax(y);
    }
}
