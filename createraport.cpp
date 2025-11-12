#include "createraport.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QApplication>
#include <QStandardPaths>

CreateRaport::CreateRaport(DataBase& data_base)
    :data_base_(data_base)
{

}


void CreateRaport::CreateAllDoc(QVector<DataSeriesACM>& vec_data){
    QString path = QApplication::applicationDirPath();
    QString dir_name = QFileDialog::getExistingDirectory(nullptr, "Сохранить контрольные точки", path);
    for (DataSeriesACM& data : vec_data){
        QString file_data ="/" + data.name_sensor.trimmed() + "_Давление.txt";
        QString file_name = dir_name +  file_data  ;
        if(file_name.isEmpty()){
            return;
        }
        CreateDoc(data,file_name);
    }
    QMessageBox::information(nullptr, "Информация","Все отчеты успешно сохранены!");
}

void CreateRaport::CreateDoc(DataSeriesACM& data, QString name){
    QVector<ACM>& acm = data.vec_acm;
    QPointer<QStandardItemModel> model_bar ;
    QPointer<QStandardItemModel> model_temp ;
    for(ACM& a :acm){
        if(a.name_chart.contains("Давление",Qt::CaseInsensitive)){
            model_bar = a.model;
        }
        if(a.name_chart.contains("Температура",Qt::CaseInsensitive)){
            model_temp = a.model;
        }
    }
    QFile raport(name);
    if(!raport.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::warning(nullptr, "Ошибка","Неудалось открыть файл для записи");
        return;
    }
    QTextStream out(&raport);
    out << "PRES	ADC_Давление(у.е.) " + data.name_sensor+ "\n";
    out << "Интерполяционная таблица по давлению\n";
    out << "------------------------------------------------\n";
    for(int row = 0; row <= model_bar->rowCount()-1; ++row){
        for(int column = 0;column <=model_bar->columnCount()-1;column++){
            QModelIndex index = model_bar->index(row,column);
            double word = model_bar->data(index).toDouble();
            if(column == 0 && row == 0){
                out << QString(15,' ')<< "|";
            } else {
                out << QString::asprintf("%15.3f|", word).replace('.',',');
                if(column == model_bar->columnCount()-1){
                    out <<"\n";
                }
            }
        }
    }
    out << "------------------------------------------------\n";
    out << "Интерполяционная таблица по температуре\n";
    out << "------------------------------------------------\n";
    for(int row = 0; row <= model_bar->rowCount()-1; ++row){
        for(int column = 0;column <=model_bar->columnCount()-1;column++){
            QModelIndex index = model_temp->index(row,column);
            double word = model_temp->data(index).toDouble();
            if(column == 0 && row == 0){
                out << QString(15,' ')<< "|";
            } else {
                out << QString::asprintf("%15.3f|", word).replace('.',',');
                if(column == model_bar->columnCount()-1){
                    out <<"\n";
                }
            }
        }
    }
    out << "------------------------------------------------\n";
}
void CreateRaport::CreateAllDeltaDoc(){
    QString path = QApplication::applicationDirPath();
    QString dir_name = QFileDialog::getExistingDirectory(nullptr, "Сохранить контрольные точки", path);
    QVector<DataSeriesACM>& data = data_base_.GetDataSerACM();
    for(DataSeriesACM& acm : data){
            QString file_data ="/" + acm.name_sensor.trimmed() + "_Погрешность.txt";
            QString file_name = dir_name +  file_data  ;
            if(file_name.isEmpty()){
                return;
            }
            CreateDeltaDoc(acm, file_name, acm.name_sensor.trimmed());
    }
}

void CreateRaport::CreateDeltaDoc(DataSeriesACM& acm,QString file_name,QString name_sensor){
    QVector<QDateTime> times = data_base_.GetCheckPoints();
    QVector<double> bar_data = data_base_.GetCheckPointBar();
    QVector<double> temp_data = data_base_.GetCheckPointTemp();
    QFile raport(file_name);
    if(!raport.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::warning(nullptr, "Ошибка","Неудалось открыть файл для записи");
        return;
    }
    QTextStream out(&raport);
    out << name_sensor + "\n";
    out << "Дата\tВремя\tTэт\tTизм\tTпогр\tPэт\tPизм\tPпогр\n";
    for(int i = 0; i<times.size();++i){
        out << times[i].toString("yyyy.MM.dd\thh-mm-ss") << "\t";
        out << QString::number(temp_data[i],'f',2) << "\t";
        if(acm.vec_acm[1].name_chart.contains("Температура",Qt::CaseInsensitive)){
            out << QString::number(acm.vec_acm[1].check_points[i], 'f',2) << "\t";
            out << QString::number(acm.vec_acm[1].delta_points[i], 'f',2) << "\t";
            out << QString::number(bar_data[i], 'f',2) << "\t";
            out << QString::number(acm.vec_acm[0].check_points[i], 'f',2) << "\t";
            out << QString::number(acm.vec_acm[0].delta_points[i], 'f',2) << "\n";
        } else {
            out << QString::number(acm.vec_acm[0].check_points[i], 'f',2) << "\t";
            out << QString::number(acm.vec_acm[0].delta_points[i], 'f',2) << "\t";
            out << QString::number(bar_data[i], 'f',2) << "\t";
            out << QString::number(acm.vec_acm[1].check_points[i], 'f',2) << "\t";
            out << QString::number(acm.vec_acm[1].delta_points[i], 'f',2) << "\n";
        }


    }
    raport.close();
}
