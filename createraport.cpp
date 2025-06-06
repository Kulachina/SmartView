#include "createraport.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QApplication>
#include <QStandardPaths>

CreateRaport::CreateRaport() {}


void CreateRaport::CreateAllDoc(QVector<DataSeriesACM>& vec_data){
    QString dir_name = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/SmartView";
    QDir dir;
    if(!dir.exists(dir_name)){
        if(!dir.mkpath(dir_name)){
            QMessageBox::warning(nullptr, "Ошибка","Неудалось создать папку для записи");
            return;
        }
    }
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
    QFile raport(name);
    if(!raport.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::warning(nullptr, "Ошибка","Неудалось открыть файл для записи");
        return;
    }
    QTextStream out(&raport);
    out << "PRES	ADC_Давление(у.е.) " + data.name_sensor+ "\n";
    out << "Интерполяционная таблица по давлению\n";
    out << "------------------------------------------------\n";
    QPointer<QStandardItemModel> model_bar = data.model_bar;
    QPointer<QStandardItemModel> model_temp = data.model_temp;
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
