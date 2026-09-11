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
    ReadConditions(in);   // необязательный хвост: в старых .sml2 его нет
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

void DowlandFile::ReadConditions(QDataStream& in) {
    char magic[4];
    if (in.readRawData(magic, sizeof(magic)) != sizeof(magic)) {
        in.resetStatus();          // старый файл: данные кончились
        return;
    }
    if (strncmp(magic, "COND", 4) != 0) {
        return;                    // хвост не наш — не трогаем
    }
    quint32 version = 0;
    QVector<double> values;
    in >> version >> values;
    if (in.status() != QDataStream::Ok) {
        in.resetStatus();          // хвост оборван — читаем как «условий нет»
        return;
    }
    if (version == 1 && values.size() == 3) {
        data_base_.GetConditions() = values;
    } else {
        qWarning() << "Условия калибровки: неизвестный формат хвоста, версия"
                   << version << "значений" << values.size();
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
    p_bar_.clear();
    p_temp_.clear();
    check_points_bar_.clear();
    check_points_temp_.clear();
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

// ---------------------------------------------------------------------------
// Документ .smv
//
// Состав: эталон (имена серий, кривые в обоих представлениях, условия,
// диапазоны осей, контрольные точки и контрольные диапазоны) и приборы
// (все каналы со всеми настройками: погрешности, допуски, цвета, выбор).
// В файл пишутся только данные; объекты Qt (серии, оси, метки) пересоздаются
// при чтении.
// ---------------------------------------------------------------------------
void DowlandFile::LoadSVDoc(const QString path, QVector<DataSeriesSensor>& out_sensors){
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(nullptr, "Ошибка", "Не удалось открыть файл для чтения:\n" + path);
        return;
    }
    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);
    in.setVersion(QDataStream::Qt_6_0);
    LoadDataEt(in);
    LoadDataACM(in, out_sensors);
    const bool ok = (in.status() == QDataStream::Ok);
    file.close();
    if(!ok){
        QMessageBox::warning(nullptr, "Ошибка",
                             "Документ прочитан не полностью — файл повреждён "
                             "или сохранён другой версией программы:\n" + path);
    }
}

void DowlandFile::LoadDataEt(QDataStream& in){
    quint32 size = 0;
    in >> size;
    QVector<DataSeriesEtalon>& etalon = data_base_.GetDataSerEtalon();
    for(quint32 i = 0;i < size && in.status() == QDataStream::Ok; ++i){
        QString name_series;
        in >> name_series;
        CreateSeriesEtalon(name_series);
        DataSeriesEtalon& doc = data_etalon_.back();
        double y_min = 0,
               y_max = 0;
        in >> doc.points_triangle_view
            >> doc.points_rectangle_view
            >> doc.condition
            >> y_min
            >> y_max;
        doc.series->replace(doc.points_rectangle_view);
        // Диапазон оси восстанавливается как есть: запас в 10% уже заложен при
        // первой загрузке эталона и повторно накручиваться не должен.
        doc.axis_y_->setRange(y_min, y_max);
        etalon.push_back(doc);
        data_base_.AddListAxis(doc.axis_y_);
    }
    qint64 min = 0,
           max = 0;
    QDateTime default_min,
              default_max;
    in >> min
        >> max
        >> default_min
        >> default_max
        >> data_base_.GetCheckPoints()
        >> data_base_.GetCheckPointTemp()
        >> data_base_.GetCheckPointBar()
        >> data_base_.GetCheckRanges()
        >> data_base_.GetConditions();
    if(in.status() != QDataStream::Ok){
        return;
    }
    RestoreCheckPoints();
    axis_x_->setRange(QDateTime::fromMSecsSinceEpoch(min),
                      QDateTime::fromMSecsSinceEpoch(max));
    if(default_min.isValid() && default_max.isValid()){
        data_base_.SetDefaultAxisX(default_max, default_min);
    } else {
        data_base_.SetDefaultAxisX(axis_x_->max(), axis_x_->min());
    }
    if(etalon.size() < 2){
        return;
    }

    // Члены min/max нужны остальным путям загрузки эталона — держим их
    // в согласии с только что прочитанным документом.
    temp_min_ = etalon[0].axis_y_->min();
    temp_max_ = etalon[0].axis_y_->max();
    bar_min_ = etalon[1].axis_y_->min();
    bar_max_ = etalon[1].axis_y_->max();
}

void DowlandFile::LoadDataACM(QDataStream& in, QVector<DataSeriesSensor>& out_sensors){
    quint32 size = 0;
    in >> size;
    for(quint32 i = 0;i < size && in.status() == QDataStream::Ok;++i){
        DataSeriesSensor data;
        quint32 size_acm = 0;
        in >> data.name_sensor
            >> data.number_sensor
            >> size_acm;
        if(in.status() != QDataStream::Ok){
            return;
        }
        data.label_sensor = new QLabel(data.name_sensor);
        data.vec_canal.resize(size_acm);
        for(quint32 j = 0;j < size_acm && in.status() == QDataStream::Ok;++j){
            Canal& acm = data.vec_canal[j];
            qint32 type_error = 0,
                   duration_error_max = 0,
                   duration_error_min = 0;
            in >> acm.name_canal
                >> acm.first_name_canal
                >> acm.new_name_canal
                >> acm.name_sensor
                >> acm.name_unit
                >> acm.color_series_
                >> acm.color_series_RGB
                >> acm.unit_max
                >> acm.unit_min
                >> acm.accept_max
                >> acm.accept_min
                >> type_error
                >> duration_error_max
                >> duration_error_min
                >> acm.select_box
                >> acm.check_ACP
                >> acm.first_unit
                >> acm.check_points
                >> acm.delta_points
                >> acm.points_rectangle
                >> acm.points_triangle;
            acm.type_error = type_error;
            acm.duration_error_max = duration_error_max;
            acm.duration_error_min = duration_error_min;
            BuildCanalWidgets(acm);
        }
        out_sensors.push_back(data);
    }
}

void DowlandFile::RestoreCheckPoints(){
    const QVector<QDateTime>& times = data_base_.GetCheckPoints();
    const QVector<double>& temp = data_base_.GetCheckPointTemp();
    const QVector<double>& bar = data_base_.GetCheckPointBar();
    // Легаси-зеркало времён: им пользуется только разбор эталона, но пусть
    // не противоречит документу.
    QVector<qint64>& times64 = data_base_.GetCheckPoints64();
    times64.clear();
    check_points_temp_.clear();
    check_points_bar_.clear();
    const qsizetype count = qMin(times.size(), qMin(temp.size(), bar.size()));
    for(qsizetype i = 0;i < count;++i){
        const qint64 t = times[i].toMSecsSinceEpoch();
        times64.push_back(t);
        check_points_temp_.push_back(QPointF(t, temp[i]));
        check_points_bar_.push_back(QPointF(t, bar[i]));
    }
    QVector<DataSeriesEtalon>& etalon = data_base_.GetDataSerEtalon();
    if(etalon.size() < 2){
        return;
    }
    etalon[0].point_series->replace(check_points_temp_);
    etalon[1].point_series->replace(check_points_bar_);
}

void DowlandFile::BuildCanalWidgets(Canal& canal){

    // Канал ещё не в легенде: hbox и check_box создаёт ChartView::SetCanal.
    canal.flag_setting_canal = false;
    canal.label = new QLabel();
    canal.label_data = new QLabel();
    canal.label_delta = new QLabel();
    canal.label_name_canal = new QLabel(canal.check_ACP ? "АЦП_" + canal.name_canal
                                                        : canal.name_canal);
    canal.label_name_sensor = new QLabel(canal.name_sensor);
    canal.axis_y_ = new QValueAxis();
    canal.axis_y_->setTickCount(21);
    QPen pen;
    pen.setWidth(1);
    canal.series = new QLineSeries();
    canal.series->setPen(pen);
    canal.series->setName(canal.name_sensor + canal.name_canal);
}

void DowlandFile::SaveSVDoc(const QString path, const QVector<DataSeriesSensor>& sensors){
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
    out.setVersion(QDataStream::Qt_6_0);
    SaveDataEt(out);
    SaveDataACM(out, sensors);
    const bool ok = (out.status() == QDataStream::Ok);
    file.close();
    if(!ok || file.error() != QFile::NoError){
        QMessageBox::warning(nullptr, "Ошибка", "Документ записан не полностью:\n" + path);
    }
}

void DowlandFile::SaveDataEt(QDataStream& out){
    QVector<DataSeriesEtalon>& etalon = data_base_.GetDataSerEtalon();
    out << static_cast<quint32>(etalon.size());
    for(DataSeriesEtalon& data : etalon){
        double y_min = 0,
               y_max = 0;
        if(data.axis_y_){
            y_min = data.axis_y_->min();
            y_max = data.axis_y_->max();
        }
        out << data.name_series
            << data.points_triangle_view
            << data.points_rectangle_view
            << data.condition
            << y_min
            << y_max;
    }
    qint64 min = 0,
           max = 0;
    if(axis_x_){
        min = axis_x_->min().toMSecsSinceEpoch();
        max = axis_x_->max().toMSecsSinceEpoch();
    }
    std::pair<QDateTime,QDateTime> default_axis = data_base_.GetDefaultAxisX();
    // Контрольные точки: времена из GetCheckPoints() — их ведут окно «КТ» и
    // перетаскивание маркера на графике. GetCheckPoints64() заполняется только
    // при разборе эталона и после правок КТ устаревает, поэтому в документ не
    // пишется. Серии маркеров не пишем тоже: они однозначно строятся из
    // времён и значений (см. ChartView::ReplaceCheckSeries).
    out << min
        << max
        << default_axis.first
        << default_axis.second
        << data_base_.GetCheckPoints()
        << data_base_.GetCheckPointTemp()
        << data_base_.GetCheckPointBar()
        << data_base_.GetCheckRanges()
        << data_base_.GetConditions();
}

void DowlandFile::SaveDataACM(QDataStream& out, const QVector<DataSeriesSensor>& sensors){
    out << static_cast<quint32>(sensors.size());
    for(const DataSeriesSensor& data : sensors){
        out << data.name_sensor
            << data.number_sensor
            << static_cast<quint32>(data.vec_canal.size());
        for(const Canal& acm : data.vec_canal){
            out << acm.name_canal
                << acm.first_name_canal
                << acm.new_name_canal
                << acm.name_sensor
                << acm.name_unit
                << acm.color_series_
                << acm.color_series_RGB
                << acm.unit_max
                << acm.unit_min
                << acm.accept_max
                << acm.accept_min
                << static_cast<qint32>(acm.type_error)
                << static_cast<qint32>(acm.duration_error_max)
                << static_cast<qint32>(acm.duration_error_min)
                << acm.select_box
                << acm.check_ACP
                << acm.first_unit
                << acm.check_points
                << acm.delta_points
                << acm.points_rectangle
                << acm.points_triangle;
        }
    }
}
