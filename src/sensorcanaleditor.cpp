#include "sensorcanaleditor.h"
#include "data_base.h"
#include "chartview.h"
#include "canalutils.h"
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QValueAxis>
#include <QLineSeries>
#include <QPen>

SensorCanalEditor::SensorCanalEditor(DataBase& data_base,
                                     ChartView* chart_view,
                                     QVector<DataSeriesSensor>& data_sensor,
                                     QVector<DataSeriesSensor>& all_data_sensor,
                                     QWidget* parent)
    : QWidget(parent),
      data_base_(data_base),
      chart_view_(chart_view),
      data_sensor_(data_sensor),
      all_data_sensor_(all_data_sensor){
    static const QMap<QString, QString> color_map = {
         {"Чёрный", "0, 0, 0"}, {"Красный", "255, 0, 0"},
         {"Темно-синий", "0, 0, 255"}, {"Синий", "68, 114, 196"},
         {"Голубой", "155, 194, 230"}, {"Фиолетовый", "112, 48, 160"},
         {"Зелёный", "0, 126, 57"}, {"Коричневый", "191, 143, 0"},
    };
    setWindowTitle("Приборы и каналы");
    resize(900, 350);
    setWindowFlags(Qt::Window
                   | Qt::WindowMinimizeButtonHint
                   | Qt::WindowMaximizeButtonHint);
    setWindowModality(Qt::ApplicationModal);
    sensor_list_ = new QListWidget();
    sensor_list_->setFixedWidth(150);
    canal_list_ = new QListWidget();
    QHBoxLayout *layout = new QHBoxLayout();
    QVBoxLayout *vlayout = new QVBoxLayout();
    QHBoxLayout *layout_btn_down = new QHBoxLayout();
    QHBoxLayout *layout_btn_up = new QHBoxLayout();
    QPushButton *btn_select = new QPushButton("Выделить все");
    QPushButton *btn_unselect = new QPushButton("Cнять все");
    layout_btn_up->setAlignment(Qt::AlignLeft);
    layout_btn_down->setAlignment(Qt::AlignLeft);
    QPushButton *btn = new QPushButton("OK");
    QPushButton *btn_close = new QPushButton("Отмена");
    connect(btn_select, &QPushButton::clicked, this, &SensorCanalEditor::CheckCanals);
    connect(btn_unselect, &QPushButton::clicked, this, &SensorCanalEditor::UncheckCanals);
    connect(btn_close, &QPushButton::clicked, this, &SensorCanalEditor::Clear);
    connect(btn, &QPushButton::clicked, this, [&](){
        if(foo_call_){
            SetPanelLegend();
        }   else{
            CreatePanelLegend();
        }
    });
    layout_btn_up->addWidget(btn_select);
    layout_btn_up->addWidget(btn_unselect);
    vlayout->addLayout(layout_btn_up);
    layout->addWidget(sensor_list_);
    layout->addWidget(canal_list_);
    vlayout->addLayout(layout);
    layout_btn_down->addWidget(btn);
    layout_btn_down->addWidget(btn_close);
    vlayout->addLayout(layout_btn_down);
    setLayout(vlayout);
    QObject::connect(sensor_list_, &QListWidget::itemSelectionChanged, this, [this]() {
        canal_list_->clear();
        if (sensor_list_->selectedItems().isEmpty()){
            return;
        }
        QString sensor_name = sensor_list_->selectedItems().first()->text();
        QVector<DataSeriesSensor>& src = foo_call_ ? all_data_sensor_ : data_sensor_;
        for (DataSeriesSensor& data : src) {
            if (data.name_sensor +" "+ data.number_sensor != sensor_name){
                continue;
            }
            for (Canal& can_ref : data.vec_canal) {
                QListWidgetItem *item = new QListWidgetItem(canal_list_);
                QWidget *w = new QWidget;
                QHBoxLayout *hbox = new QHBoxLayout(w);
                QCheckBox* check = new QCheckBox(can_ref.first_name_canal);
                can_ref.check_active_canal = check;
                QComboBox* combo_color = new QComboBox;
                QComboBox* combo_name = new QComboBox;
                QComboBox* combo_error = new QComboBox;
                QComboBox* combo_accept = new QComboBox;
                QComboBox* combo_unit = new QComboBox;
                QComboBox* combo_duration_min = new QComboBox;
                QComboBox* combo_duration_max = new QComboBox;
                QCheckBox* check_ACP = new QCheckBox("AЦП");
                combo_name->addItems(chanels_);
                combo_duration_max->addItems({"0","250","400","600","1000",""});
                combo_duration_min->addItems({"0","250","400","600","1000",""});
                combo_color->addItems({"Чёрный","Красный","Синий","Голубой","Фиолетовый","Зелёный","Темно-синий","Коричневый"});
                combo_error->addItems({" ","Абсолютная","Относительная","Приведённая"});
                combo_error->setCurrentText(" ");
                combo_accept->addItems({"0.5","1","1.5","0.15",""});
                combo_unit->addItems({"кгс/см2","МПа","°C","К","мкР/ч","м3/ч",
                                      "См/м","мСм/см","%","-","г/см3","кг/м3","усл.ед"});
                item->setSizeHint(QSize(400, 40));
                hbox->setContentsMargins(0,0,0,0);
                check->setCheckState(Qt::Unchecked);
                check_ACP->setCheckState(Qt::Unchecked);
                combo_duration_max->setEditable(true);
                combo_duration_min->setEditable(true);
                combo_name->setEnabled(false);
                check_ACP->setEnabled(false);
                combo_unit->setEnabled(false);
                combo_error->setEnabled(false);
                combo_accept->setEnabled(false);
                combo_duration_min->setEnabled(false);
                combo_duration_max->setEnabled(false);
                combo_color->setEnabled(false);
                connect(combo_duration_max,&QComboBox::currentIndexChanged, w,[can_ptr = &can_ref, combo_duration_max](const int index){
                    if(combo_duration_max->count() - 1 == index){
                        combo_duration_max->lineEdit()->setReadOnly(false);
                        combo_duration_max->lineEdit()->clear();
                    } else {
                        combo_duration_max->lineEdit()->setReadOnly(true);
                        can_ptr->duration_error_max = combo_duration_max->currentText().toDouble();
                    }
                });
                connect(combo_duration_max->lineEdit(), &QLineEdit::editingFinished,  [can_ptr = &can_ref, combo_duration_max](){
                    bool ok = false;
                    double val = combo_duration_max->lineEdit()->text().toDouble(&ok);
                    if(ok){
                        can_ptr->duration_error_max = val;
                    }
                });
                connect(combo_duration_min,&QComboBox::currentIndexChanged, w,[can_ptr = &can_ref, combo_duration_min](const int index){
                    if(combo_duration_min->count() - 1 == index){
                        combo_duration_min->lineEdit()->setReadOnly(false);
                        combo_duration_min->lineEdit()->clear();
                    } else {
                        combo_duration_min->lineEdit()->setReadOnly(true);
                        can_ptr->duration_error_min = combo_duration_min->currentText().toDouble();
                    }
                });
                connect(combo_duration_min->lineEdit(), &QLineEdit::editingFinished,  [can_ptr = &can_ref, combo_duration_min](){
                    bool ok = false;
                    double val = combo_duration_min->lineEdit()->text().toDouble(&ok);
                    if(ok){
                        can_ptr->duration_error_min = val;
                    }
                });
                combo_accept->setEditable(true);
                connect(combo_accept,&QComboBox::currentIndexChanged, w,[can_ptr = &can_ref, combo_accept](const int index){
                    if(combo_accept->count() - 1 == index){
                        combo_accept->lineEdit()->setReadOnly(false);
                        combo_accept->lineEdit()->clear();
                    } else {
                        combo_accept->lineEdit()->setReadOnly(true);
                        can_ptr->accept_max = combo_accept->currentText().toDouble();
                        can_ptr->accept_min = combo_accept->currentText().toDouble() * (-1);
                    }
                });
                connect(combo_accept->lineEdit(), &QLineEdit::editingFinished,  [can_ptr = &can_ref, combo_accept](){
                    bool ok = false;
                    double val = combo_accept->lineEdit()->text().toDouble(&ok);
                    if(ok){
                        can_ptr->accept_max = val;
                        can_ptr->accept_min = val * (-1);
                    }

                });
                connect(combo_error, &QComboBox::currentTextChanged, w, [can_ptr = &can_ref,combo_error, combo_duration_max, combo_duration_min]{
                    if(combo_error->currentText() == "Абсолютная"){
                        can_ptr->type_error = 1;
                        combo_duration_max->setEnabled(false);
                        combo_duration_min->setEnabled(false);
                    }
                    if(combo_error->currentText() == "Относительная"){
                        can_ptr->type_error = 2;
                        combo_duration_max->setEnabled(false);
                        combo_duration_min->setEnabled(false);
                    }
                    if(combo_error->currentText() == "Приведённая"){
                        can_ptr->type_error = 3;
                        combo_duration_max->setEnabled(true);
                        combo_duration_min->setEnabled(true);
                    }
                });
                connect(check, &QCheckBox::checkStateChanged, w,[can_ptr = &can_ref,check,combo_name,check_ACP,combo_unit,combo_error,combo_accept,combo_duration_min,combo_duration_max,combo_color](){
                    if(check->isChecked()){
                        can_ptr->select_box = true;
                        combo_name->setEnabled(true);
                        check_ACP->setEnabled(true);
                        combo_unit->setEnabled(true);
                        combo_error->setEnabled(true);
                        combo_accept->setEnabled(true);
                        combo_duration_min->setEnabled(true);
                        combo_duration_max->setEnabled(true);
                        combo_color->setEnabled(true);
                    } else {
                        combo_name->setEnabled(false);
                        check_ACP->setEnabled(false);
                        check_ACP->setCheckState(Qt::Unchecked);
                        combo_unit->setEnabled(false);
                        combo_error->setEnabled(false);
                        combo_accept->setEnabled(false);
                        combo_duration_min->setEnabled(false);
                        combo_duration_max->setEnabled(false);
                        combo_color->setEnabled(false);
                        can_ptr->select_box = false;
                    }
                });
                connect(check_ACP, &QCheckBox::checkStateChanged, w,[can_ptr = &can_ref,check_ACP,combo_unit,combo_error,combo_accept,combo_duration_min,combo_duration_max](){
                    if(check_ACP->isChecked() && can_ptr->select_box){
                        combo_unit->setCurrentText("-");
                        can_ptr->name_unit = "-";
                        combo_unit->setEnabled(false);
                        can_ptr->check_ACP = true;
                        combo_error->setEnabled(false);
                        combo_accept->setEnabled(false);
                        combo_duration_min->setEnabled(false);
                        combo_duration_max->setEnabled(false);
                    } else {
                        combo_unit->setEnabled(true);
                        can_ptr->check_ACP = false;
                        combo_error->setEnabled(true);
                        combo_accept->setEnabled(true);
                        combo_duration_min->setEnabled(true);
                        combo_duration_max->setEnabled(true);
                    }
                });
                connect(combo_color, &QComboBox::currentTextChanged, w, [can_ptr = &can_ref](const QString &text){
                    can_ptr->color_series_RGB = color_map.value(text);
                    can_ptr->color_series_ = text;
                });
                connect(combo_name, &QComboBox::currentTextChanged, w, [can_ptr = &can_ref](const QString &text){
                    can_ptr->new_name_canal =  can_ptr->name_canal;
                    can_ptr->name_canal =  text;
                });
                connect(combo_unit, &QComboBox::currentTextChanged, w, [can_ptr = &can_ref](const QString &text){
                    can_ptr->name_unit = text;
                });
                if(can_ref.select_box){
                    check->setCheckState(Qt::Checked);
                    int ind =  combo_name->findText(can_ref.name_canal);
                    if(ind == -1){
                        QString name = can_ref.name_canal;
                        combo_name->setCurrentText(name.mid(4));
                    } else {
                        combo_name->setCurrentText(can_ref.name_canal);
                    };
                    if(can_ref.check_ACP){
                        check_ACP->setCheckState(Qt::Checked);
                        combo_unit->setCurrentText("-");
                        combo_unit->setEnabled(false);
                        combo_error->setEnabled(false);
                        combo_accept->setEnabled(false);
                        combo_duration_min->setEnabled(false);
                        combo_duration_max->setEnabled(false);
                    } else {
                        check_ACP->setCheckState(Qt::Unchecked);
                        combo_unit->setEnabled(true);
                        combo_error->setEnabled(true);
                        combo_accept->setEnabled(true);
                        combo_duration_min->setEnabled(true);
                        combo_duration_max->setEnabled(true);
                    }
                    if(can_ref.type_error == 1){
                        combo_error->setCurrentText("Абсолютная");
                        combo_duration_min->setEnabled(false);
                        combo_duration_max->setEnabled(false);
                    }
                    if(can_ref.type_error == 2){
                        combo_error->setCurrentText("Относительная");
                        combo_duration_min->setEnabled(false);
                        combo_duration_max->setEnabled(false);
                    }
                    if(can_ref.type_error == 3){
                        combo_error->setCurrentText("Приведённая");
                        combo_duration_min->setEnabled(true);
                        combo_duration_max->setEnabled(true);
                        combo_duration_min->setCurrentText(QString::number(can_ref.duration_error_min));
                        combo_duration_max->setCurrentText(QString::number(can_ref.duration_error_max));
                    }
                    combo_unit->setCurrentText(can_ref.name_unit);
                    combo_accept->setCurrentText(QString::number(can_ref.accept_max));
                    combo_color->setCurrentText(can_ref.color_series_);
                } else {
                    if(can_ref.name_canal.contains("Температура",Qt::CaseInsensitive)){
                        combo_name->setCurrentText("Т-Температура");
                        combo_unit->setCurrentText("°C");
                        combo_color->setCurrentText("Синий");
                    }
                    if(can_ref.name_canal.contains("Давление",Qt::CaseInsensitive)){
                        combo_name->setCurrentText("Р-Давление");
                        combo_unit->setCurrentText("кгс/см2");
                        combo_color->setCurrentText("Красный");
                    }
                    can_ref.select_box = false;
                    can_ref.accept_min = -1.5;
                    can_ref.accept_max = 1.5;

                    combo_color->setCurrentText("Черный");
                }
                hbox->addWidget(check);
                hbox->addWidget(combo_name);
                hbox->addWidget(check_ACP);
                hbox->addWidget(combo_unit);
                hbox->addWidget(combo_error);
                hbox->addWidget(new QLabel("Допуск ±"));
                hbox->addWidget(combo_accept);
                hbox->addWidget(new QLabel("Диапазон"));
                hbox->addWidget(combo_duration_min);
                hbox->addWidget(combo_duration_max);
                hbox->addWidget(combo_color);
                w->setLayout(hbox);
                canal_list_->setItemWidget(item, w);
            }
        }
    });
}

void SensorCanalEditor::PopulateAndShow(QVector<DataSeriesSensor>& vec_data){
    sensor_list_->clear();
    for(DataSeriesSensor&  data : vec_data){
        sensor_list_->addItem(data.name_sensor +" "+ data.number_sensor);
    }
    show();
}

void SensorCanalEditor::OpenForNew(){
    foo_call_ = false;
    PopulateAndShow(data_sensor_);
}

void SensorCanalEditor::OpenForEdit(){
    foo_call_ = true;
    PopulateAndShow(all_data_sensor_);
}

void SensorCanalEditor::Clear(){
    data_sensor_.clear();
    hide();
}

void SensorCanalEditor::CreatePanelLegend(){
    for(auto& data : data_sensor_){
        chart_view_->PanelLegendACM(data);
        data_base_.AddDataSerACM(data);
        all_data_sensor_.push_back(data);
    }
    data_sensor_.clear();
    hide();
}

void SensorCanalEditor::SetPanelLegend(){
    for(auto& data : all_data_sensor_){
        bool active_sens = false;
        if(!data_base_.FindSensor(data.name_sensor)){
            for(auto& canal : data.vec_canal){
                RebuildCanal(canal);
            }
            chart_view_->PanelLegendACM(data);
            data_base_.AddDataSerACM(data);
            continue;
        }
        for(auto& canal : data.vec_canal){
            QVector<DataSeriesSensor>& data_acm =  data_base_.GetDataSerACM();
            if(canal.select_box && (data_base_.FindCanal(data.name_sensor, canal.name_canal) || data_base_.FindCanal(data.name_sensor, canal.new_name_canal))){
                for(auto& d : data_acm){
                    if(d.name_sensor == data.name_sensor){
                        for(auto& can : d.vec_canal){
                            if(can.name_canal == canal.new_name_canal){
                                can.duration_error_max = canal.duration_error_max;
                                can.duration_error_min = canal.duration_error_min;
                                can.name_unit = canal.name_unit;
                                can.name_canal = canal.name_canal;
                                can.color_series_ = canal.color_series_;
                                can.color_series_RGB = canal.color_series_RGB;
                                can.select_box = canal.select_box;
                                can.type_error = canal.type_error;
                                can.accept_max = canal.accept_max;
                                can.accept_min = canal.accept_min;
                                can.axis_y_->setTitleText(can.name_sensor + " " + can.name_canal +" "+can.name_unit);
                                if(canal.check_ACP){
                                    can.check_ACP = true;
                                    can.label_name_canal->setText("АЦП_" + can.name_canal);
                                } else {
                                    can.check_ACP = false;
                                    can.label_name_canal->setText(can.name_canal);
                                }
                                chart_view_->SetCanal(can);
                            }
                        }
                    }
                }
                active_sens = true;
                continue;
            }
            if(canal.select_box && !data_base_.FindCanal(data.name_sensor, canal.name_canal)){
                for(auto& d : data_acm){
                    if(d.name_sensor == data.name_sensor){
                        d.vec_canal.push_back(canal);
                        RebuildCanal(d.vec_canal.back());
                        chart_view_->SetCanal(d.vec_canal.back());
                    }
                }
            }
            if(!canal.select_box && data_base_.FindCanal(data.name_sensor, canal.name_canal)){
                for(auto& d : data_acm){
                    if(d.name_sensor == data.name_sensor){
                        for(auto it = d.vec_canal.begin();it != d.vec_canal.end();){
                            if(canal.name_canal == it->name_canal){
                                DeleteCanal(*it);
                                it = d.vec_canal.erase(it);
                            } else {
                                ++it;
                            }
                        }
                    }
                }
            }
            if(canal.select_box){
                active_sens = true;
            }
        }
        if(!active_sens){
            for(auto iter = data_base_.GetDataSerACM().begin();iter != data_base_.GetDataSerACM().end();){
                if(iter->name_sensor == data.name_sensor){
                    //DeleteSens(*iter);
                    iter = data_base_.GetDataSerACM().erase(iter);
                } else {
                    ++iter;
                }
            }
        }
    }
    chart_view_->ReBuildMapLbelndSeries();
    hide();
}

void SensorCanalEditor::RebuildCanal(Canal& canal){
    canal.flag_setting_canal = false;
    canal.label_data = new QLabel();
    canal.label_name_canal = new QLabel(canal.name_canal);
    canal.label_name_sensor = new QLabel(canal.name_sensor);
    QPen pen;
    pen.setWidth(1);
    canal.axis_y_ = new QValueAxis();
    canal.axis_y_->setTickCount(21);
    canal.series = new QLineSeries();
    canal.series->setPen(pen);
    canal.series->setName(canal.name_sensor + canal.name_canal);
    canal.label = new QLabel();
    canal.label_delta = new QLabel();
    canal.hbox = new QHBoxLayout();
}

void SensorCanalEditor::CheckCanals(){
    DataSeriesSensor* data = PreparationData();
    if(data){
        for(Canal& canal : data->vec_canal)
            canal.check_active_canal->setCheckState(Qt::Checked);
    }
}

void SensorCanalEditor::UncheckCanals(){
    DataSeriesSensor* data = PreparationData();
    if(data){
        for(Canal& canal : data->vec_canal){
            canal.check_active_canal->setCheckState(Qt::Unchecked);
        }
    }
}

DataSeriesSensor* SensorCanalEditor::PreparationData(){
    if(!sensor_list_->selectedItems().isEmpty()){
        QString sensor_name = sensor_list_->selectedItems().first()->text();
        QVector<DataSeriesSensor>* data_sens = nullptr;
        if(foo_call_){
            data_sens = &all_data_sensor_;
        } else {
            data_sens = &data_sensor_;
        }
        for (DataSeriesSensor& data : *data_sens)
            if (data.name_sensor +" "+ data.number_sensor != sensor_name){
                continue;
            } else {
                return &data;
            }
    }
    return nullptr;
}
