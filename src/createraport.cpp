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


void CreateRaport::CreateAllDoc(QVector<DataSeriesSensor>& vec_data){
    QString path = QApplication::applicationDirPath();
    QString dir_name = QFileDialog::getExistingDirectory(nullptr, "Сохранить контрольные точки", path);
    for (DataSeriesSensor& data : vec_data){
        QString file_data ="/" + data.name_sensor.trimmed() + "_Давление.txt";
        QString file_name = dir_name +  file_data  ;
        if(file_name.isEmpty()){
            return;
        }
        CreateDoc(data,file_name);
    }
    QMessageBox::information(nullptr, "Информация","Все отчеты успешно сохранены!");
}
void CreateRaport::CreateAllShortDoc(QVector<DataSeriesSensor>& vec_data,QString name_canal){
    QString path = QApplication::applicationDirPath();
    QString dir_name = QFileDialog::getExistingDirectory(nullptr, "Сохранить контрольные точки", path);
    for (DataSeriesSensor& data : vec_data){
        QString file_data ="/" + data.name_sensor.trimmed() + "_" + name_canal + ".txt";
        QString file_name = dir_name +  file_data  ;
        if(file_name.isEmpty()){
            return;
        }
        CreateShortDoc(data,file_name,name_canal);
    }
    QMessageBox::information(nullptr, "Информация","Все отчеты успешно сохранены!");
}

void CreateRaport::CreateDoc(DataSeriesSensor& data, QString name){
    QVector<Canal>& acm = data.vec_canal;
    QPointer<QStandardItemModel> model_bar ;
    QPointer<QStandardItemModel> model_temp ;
    for(Canal& a :acm){
        if(a.name_canal.contains("Давление",Qt::CaseInsensitive)){
            if(!a.model){
                continue;
            }
            model_bar = a.model;
        }
        if(a.name_canal.contains("Температура",Qt::CaseInsensitive)){
            if(!a.model){
                continue;
            }
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
    if(!model_bar.isNull()){
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
    }
    out << "------------------------------------------------\n";
    out << "Интерполяционная таблица по температуре\n";
    out << "------------------------------------------------\n";
    if(!model_temp.isNull()){
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
    }
    out << "------------------------------------------------\n";
}
void CreateRaport::CreateShortDoc(DataSeriesSensor& data, QString name,QString name_canal){
    QVector<Canal>& acm = data.vec_canal;
    QPointer<QStandardItemModel> model ;
    for(Canal& a :acm){
        if(a.name_canal.contains(name_canal,Qt::CaseInsensitive)){
            if(!a.model){
                continue;
            }
            model = a.model;
        }
    }
    QFile raport(name);
    if(!raport.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::warning(nullptr, "Ошибка","Неудалось открыть файл для записи");
        return;
    }
    QTextStream out(&raport);
    out << "TEMP	ADC_Температура(град) " + data.name_sensor+ "\n";
    out << QString("%1").arg("№ точки",18)
        << QString("%1").arg("Время",16)
        << QString("%1").arg("Значение",16)
        << QString("%1").arg("Эталон",16)
        << "\n";
    out << QString(66,'-') <<"\n";
    if(!model.isNull()){
        for(int row = 1; row <= model->rowCount()-1; ++row){
            for(int column = 0;column <=model->columnCount()-1;column++){
                QModelIndex index = model->index(row,column);
                if(column > 1){
                    double word = model->data(index).toDouble();
                    out << QString::asprintf("%16.2f", word).replace('.',',');
                }
                if(column == 0){
                    QString word = model->data(index).toString();
                    out << QString("%1").arg(word,18);
                }
                if(column == 1){
                    QString word = model->data(index).toString();
                    out << QString("%1").arg(word,16);
                }
                if(column == model->columnCount()-1){
                    out <<"\n";
                }
            }
        }
    }
    out << QString(66,'-') <<"\n";
}
void CreateRaport::CreateAllDocLAS(QVector<DataSeriesSensor>& vec_data,QString name_canal, QString date){
    QString path = QApplication::applicationDirPath();
    QString dir_name = QFileDialog::getExistingDirectory(nullptr, "Сохранить контрольные точки", path);
    for (DataSeriesSensor& data : vec_data){
        QString file_data ="/" + data.name_sensor.trimmed() + "_PT_" + date + ".csv";
        QString file_name = dir_name +  file_data ;
        if(file_name.isEmpty()){
            return;
        }
        CreateDocLAS(data,file_name,name_canal, date);
    }
    QMessageBox::information(nullptr, "Информация","Все отчеты успешно сохранены!");
}
void CreateRaport::CreateDocLAS(DataSeriesSensor& data, QString name,QString name_canal, QString date){
    QVector<Canal>& acm = data.vec_canal;
    QPointer<QStandardItemModel> model ;
    for(Canal& a :acm){
        if(a.name_canal.contains(name_canal,Qt::CaseInsensitive)){
            if(!a.model){
                continue;
            }
            model = a.model;
        }
    }
    QFile raport(name);
    if(!raport.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::warning(nullptr, "Ошибка","Неудалось открыть файл для записи");
        return;
    }
    QTextStream out(&raport);
    if(!model.isNull()){
        out << data.name_sensor << ";" << date << ";" << "\n";
        for(int row = 0; row <= model->rowCount()-1; ++row){
            for(int column = 0;column <=model->columnCount()-1;column++){
                QModelIndex index = model->index(row,column);
                QString word = model->data(index).toString();
                if(column == model->columnCount()-1){
                    out <<"\n";
                    continue;
                }
                word.replace('.',',');
                out << word << ";";
            }
        }
    }
    raport.close();
}
