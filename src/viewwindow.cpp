#include "viewwindow.h"
#include "data_base.h"
#include "chartview.h"
#include <QChart>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>

ViewWindow::ViewWindow(DataBase& data_base, ChartView* chart_view, QWidget* parent)
    : QWidget(parent), data_base_(data_base), chart_view_(chart_view){
    setWindowTitle("Вид");
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::WindowCloseButtonHint);
    QCheckBox *ch_auto = new QCheckBox("Автомаштаб");
    connect(ch_auto, &QCheckBox::toggled, this, &ViewWindow::AutoZoom);
    QGroupBox *gr_box = new QGroupBox("Вид отображения кривых");
    QVBoxLayout *gr_vbox = new QVBoxLayout();
    QVBoxLayout *vbox = new QVBoxLayout(this);
    QRadioButton *rad_1 = new QRadioButton("В виде прямоугольников");
    connect(rad_1, &QRadioButton::clicked, this, &ViewWindow::ReplaceRectangle);
    QRadioButton *rad_2 = new QRadioButton("В виде треугольников");
    connect(rad_2, &QAbstractButton::clicked, this, &ViewWindow::ReplaceTriangle);
    rad_1->setChecked(true);
    gr_vbox->setAlignment(Qt::AlignLeft);
    gr_vbox->addWidget(rad_1);
    gr_vbox->addWidget(rad_2);
    gr_box->setLayout(gr_vbox);
    vbox->addWidget(ch_auto);
    vbox->addWidget(gr_box);
}

void ViewWindow::AutoZoom(){
    chart_view_->AutoZoom();
}

void ViewWindow::ReplaceTriangle(){
    if(!data_base_.GetDataSerACM().isEmpty()){
        for(const DataSeriesSensor& data : data_base_.GetDataSerACM()){
            const QVector<Canal>& acm = data.vec_canal;
            for(const Canal& a : acm){
                a.series->replace(a.points_triangle);
            }
        }
    }
    if(!data_base_.GetDataSerEtalon().isEmpty()){
        DataSeriesEtalon& data_temp = data_base_.GetDataSerEtalon()[0];
        data_base_.GetDataSerEtalon()[0].series->replace(data_temp.points_triangle_view);
        DataSeriesEtalon& data_bar = data_base_.GetDataSerEtalon()[1];
        data_base_.GetDataSerEtalon()[1].series->replace(data_bar.points_triangle_view);
    }
    chart_view_->GetChart()->update();
}

void ViewWindow::ReplaceRectangle(){
    if(!data_base_.GetDataSerACM().isEmpty()){
        for(const DataSeriesSensor& data : data_base_.GetDataSerACM()){
            const QVector<Canal>& acm = data.vec_canal;
            for(const Canal& a : acm){
                a.series->replace(a.points_rectangle);
            }
        }
    }
    if(!data_base_.GetDataSerEtalon().isEmpty()){
        DataSeriesEtalon& data_temp = data_base_.GetDataSerEtalon()[0];
        data_base_.GetDataSerEtalon()[0].series->replace(data_temp.points_rectangle_view);
        DataSeriesEtalon& data_bar = data_base_.GetDataSerEtalon()[1];
        data_base_.GetDataSerEtalon()[1].series->replace(data_bar.points_rectangle_view);
    }
    chart_view_->GetChart()->update();
}
