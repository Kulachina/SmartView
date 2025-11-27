#include "dowland_file.h"
#include <QVBoxLayout>
#include <QtMath>
#include <QPushButton>

DowlandFile::DowlandFile(DataBase& data_base)
    : data_base_(data_base)
{
    w_progress_ = new QWidget();
    w_select_chart_ = new QWidget();
    w_progress_->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    w_progress_->setWindowTitle("Загрузка...");
    QVBoxLayout *vb = new QVBoxLayout();
    progress_ = new QProgressBar();
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
    SelectChart(line_text);
    progress_->setRange(0,size);
    progress_->setValue(0);
    w_progress_->show();
    CreateSeriesACM();
    int i = line_text.size()-2;
    words = line_text[i].split(" ",Qt::SkipEmptyParts);
    axis_x_->setMax(QDateTime::fromMSecsSinceEpoch(TextToInt(words[0]+ " " +words[1])));
    for(int i = index_data_;i < line_text.size()-1; ++i){
        progress_->setValue(i);
        QCoreApplication::processEvents();
        words = line_text[i].split(" ",Qt::SkipEmptyParts);
        AddDataACM(words);
    }
    w_progress_->hide();
    DataSeriesSensor& data = data_base_.GetDataSerACM().back();
    QVector<Canal>& acm = data.vec_canal;
    for(int i =0; i < acm.size();++i){
        acm[i].series->replace(acm[i].points_triangle);
        if(acm[i].name_chart.contains("Давление",Qt::CaseInsensitive)){
            axis_bar_->setRange(acm[i].unit_min,acm[i].unit_max +acm[i].unit_max * 0.1);
        }
        if(acm[i].name_chart.contains("Температура",Qt::CaseInsensitive)){
           axis_temp_->setRange(acm[i].unit_min,acm[i].unit_max +acm[i].unit_max * 0.1);
        }
    }
    chart_name_and_index_.clear();
}
void DowlandFile::SelectChart(QStringList& words){
    QDialog dil;
    QStringList word;
    word = words[0].split(" ",Qt::SkipEmptyParts);
    if(!word[4].isEmpty()){
        dil.setWindowTitle(word[4] +" "+ word[5]);
    }
    QVBoxLayout *vbox = new QVBoxLayout();
    QDialogButtonBox *btn = new QDialogButtonBox(QDialogButtonBox::Ok);
    if(!check_list_.isEmpty()){
        check_list_.clear();
    }
    for(int i = 0;i< words.size()-1;++i){
        if(words[i].isEmpty() || words[i] == "\r"){
            index_data_ = i+1;
            break;
        }
        word = words[i].split(" ",Qt::SkipEmptyParts);
        QCheckBox *check = new QCheckBox(word[0] + " " + (word[1].mid(1,word[1].size()-3)));
        check->setChecked(true);
        check_list_.append(check);
        vbox->addWidget(check);
    }
    QObject::connect(btn,&QDialogButtonBox::accepted,&dil,&QDialog::accept);
    vbox->addWidget(btn);
    dil.setLayout(vbox);
    dil.accept();
    //if(dil.exec() == QDialog::Accepted){
        for(int i=0;i < check_list_.size();++i){
            if(check_list_[i]->isChecked()){
                word = words[i].split(" ",Qt::SkipEmptyParts);
                chart_name_and_index_.push_back({check_list_[i]->text(), i +2,word[1],word[2],word[4] +" "+ word[5],0,0,true});
            }
        }
    //};
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
    data_etalon_[1].axis_y_->setRange(bar_min_,bar_max_ + bar_max_ * 0.1);
    data_etalon_[0].axis_y_->setRange(temp_min_,temp_max_ + temp_max_ * 0.1);
    data_base_.SetDefaultAxisX(axis_x_->max(),axis_x_->min());
    data_base_.GetDataSerEtalon()[0] = data_etalon_[0];
    data_base_.GetDataSerEtalon()[1] = data_etalon_[1];
    data_base_.GetDataSerEtalon()[1].points_triangle_view_bar = p_bar_;
    data_base_.GetDataSerEtalon()[0].points_triangle_view_temp = p_temp_;
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
void DowlandFile::CreateSeriesACM(){
    DataSeriesSensor data;
    data.name_sensor = chart_name_and_index_[0].name_sensor;
    data.label_sensor = new QLabel(data.name_sensor);
    for(NameChart name_chart : chart_name_and_index_){
        CreateACM(data, name_chart.name, name_chart.name_type.mid(1,name_chart.name_type.size()-3), name_chart.name_unit);
    }
    data_acm_.push_back(data);
    data_base_.AddDataSerACM(data_acm_.back());
}
void DowlandFile::CreateACM(DataSeriesSensor& data, QString name, QString name_type, QString name_unit){
    Canal acm;
    acm.label_data = new QLabel();
    acm.name_chart = name;
    acm.name_type = name_type;
    acm.name_unit = name_unit;
    QPen pen;
    pen.setWidth(1);
    acm.series = new QLineSeries();
    acm.series->setPen(pen);
    acm.series->setName(data.name_sensor + name);
    chart_->addSeries(acm.series);
    if(name.contains("Давление",Qt::CaseInsensitive)){
        acm.series->attachAxis(axis_bar_);
        acm.series->setColor("red");
        acm.series->setObjectName("bar");
    }
    if(name.contains("Температура",Qt::CaseInsensitive)){
        acm.series->attachAxis(axis_temp_);
        acm.series->setColor("blue");
        acm.series->setObjectName("temp");
    }
    acm.label = new QLabel();
    acm.label_delta = new QLabel();
    acm.series->attachAxis(axis_x_);
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
    data_base_.AddDataSerEtalon(data_etalon_.back());
    data_base_.AddListAxis(data_etalon_.back().axis_y_);
}
void DowlandFile::AddDataACM(QStringList words){
    DataSeriesSensor& data =  data_base_.GetDataSerACM().back();
    QVector<Canal>& vec_canal = data.vec_canal;
    qint64 time = TextToInt(words[0]+ " " + words[1]);
    for(int i =0; i < vec_canal.size();++i){
        words[i+2].replace(',','.');
        double num = words[i+2].toDouble();
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
    QVector<QPointF>& points_bar = data_etalon_[1].points_rectangle_view_bar;
    QVector<QPointF>& points_temp = data_etalon_[0].points_rectangle_view_temp;
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
    bar_min_ = qMin(bar_min_,bar);
    bar_max_ = qMax(bar_max_,bar);
    temp_max_ = qMax(temp_max_,temp);
    temp_min_ = qMin(temp_min_,temp);
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
        in >> data_base_.GetDataSerEtalon()[i].points_triangle_view_bar
            >> data_base_.GetDataSerEtalon()[i].points_rectangle_view_bar
            >> data_base_.GetDataSerEtalon()[i].points_triangle_view_temp
            >> data_base_.GetDataSerEtalon()[i].points_rectangle_view_temp;
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
    data_base_.GetDataSerEtalon()[1].series->replace(data_base_.GetDataSerEtalon()[1].points_rectangle_view_bar);
    data_base_.GetDataSerEtalon()[0].series->replace(data_base_.GetDataSerEtalon()[0].points_rectangle_view_temp);
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
            in >> acm.name_chart
                >> acm.name_type
                >> acm.name_unit
                >> acm.unit_max
                >> acm.unit_min
                >> acm.check_points
                >> acm.delta_points
                >> acm.points_rectangle
                >> acm.points_triangle;
            acm.label_data = new QLabel();
            QPen pen;
            pen.setWidth(1);
            acm.series = new QLineSeries();
            acm.series->setPen(pen);
            acm.series->setName(data.name_sensor + acm.name_chart);
            chart_->addSeries(acm.series);
            acm.series->replace(acm.points_rectangle);
            if(acm.name_chart.contains("Давление",Qt::CaseInsensitive)){
                acm.series->attachAxis(axis_bar_);
                acm.series->setColor("red");
                acm.series->setObjectName("bar");
            }
            if(acm.name_chart.contains("Температура",Qt::CaseInsensitive)){
                acm.series->attachAxis(axis_temp_);
                acm.series->setColor("blue");
                acm.series->setObjectName("temp");
            }
            acm.label = new QLabel();
            acm.label_delta = new QLabel();
            acm.series->attachAxis(axis_x_);
            if(acm.name_chart.contains("Давление",Qt::CaseInsensitive)){
                axis_bar_->setRange(acm.unit_min,acm.unit_max +acm.unit_max * 0.1);
            }
            if(acm.name_chart.contains("Температура",Qt::CaseInsensitive)){
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
           out << data.points_triangle_view_bar
            << data.points_rectangle_view_bar
            << data.points_triangle_view_temp
            << data.points_rectangle_view_temp;
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
            out << acm.name_chart
                << acm.name_type
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

