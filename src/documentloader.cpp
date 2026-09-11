#include "documentloader.h"
#include "data_base.h"
#include "dowland_file.h"
#include "las.h"
#include "chartview.h"
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

DocumentLoader::DocumentLoader(DataBase& data_base,
                               DowlandFile& dow_file,
                               Las& las,
                               ChartView* chart_view,
                               QVector<DataSeriesSensor>& all_sensors,
                               QWidget* dialog_parent)
    : data_base_(data_base),
      dow_file_(dow_file),
      las_(las),
      chart_view_(chart_view),
      all_sensors_(all_sensors),
      dialog_parent_(dialog_parent){}

QVector<DataSeriesSensor> DocumentLoader::LoadACM(){
    QStringList path_doc;
    if(!first_open_doc_){
        QString path = QApplication::applicationDirPath();
        path_doc = QFileDialog::getOpenFileNames(dialog_parent_, "Открытие файла", path ,"Текстовый документ (*.txt)");
        first_open_doc_ = true;
    } else {
        path_doc = QFileDialog::getOpenFileNames(dialog_parent_, "Открытие файла", save_path_ ,"Текстовый документ (*.txt)");
    }
    QVector<DataSeriesSensor> result;
    if(path_doc.isEmpty()){
        return result;
    }
    int count_now = 1;
    const int count_file = path_doc.size();
    for(const QString& path : path_doc){
        if(path.endsWith(".txt", Qt::CaseInsensitive)){
            DataSeriesSensor data = dow_file_.LoadDocACM(path, count_file, count_now);
            if(data.name_sensor.isEmpty()){
                QMessageBox::critical(nullptr,"Ошибка", "Ошибка чтения заголовка, файла " + QString::number(count_now));
                ++count_now;
                continue;
            }
            result.push_back(data);
            QFileInfo file_info(path);
            save_path_ = file_info.absolutePath();
            ++count_now;
        }
    }
    return result;
}

QVector<DataSeriesSensor> DocumentLoader::LoadLAS(){
    QStringList path_doc;
    if(!first_open_doc_){
        QString path = QApplication::applicationDirPath();
        path_doc = QFileDialog::getOpenFileNames(dialog_parent_, "Открытие файла", path ,"Las (*.las)");
        first_open_doc_ = true;
    } else {
        path_doc = QFileDialog::getOpenFileNames(dialog_parent_, "Открытие файла", save_path_ ,"Las (*.las)");
    }
    QVector<DataSeriesSensor> result;
    if(path_doc.isEmpty()){
        return result;
    }
    for(const QString& path : path_doc){
        if(path.endsWith(".las", Qt::CaseInsensitive)){
            DataSeriesSensor data = las_.DowlandLas(path);
            result.push_back(data);
            QFileInfo file_info(path);
            save_path_ = file_info.absolutePath();
        }
    }
    return result;
}

QVector<DataSeriesSensor> DocumentLoader::LoadAMT(){
    QStringList path_doc;
    if(!first_open_doc_){
        QString path = QApplication::applicationDirPath();
        path_doc = QFileDialog::getOpenFileNames(dialog_parent_, "Открытие файла", path ,"Текстовый документ (*.txt);");
        first_open_doc_ = true;
    } else {
        path_doc = QFileDialog::getOpenFileNames(dialog_parent_, "Открытие файла", save_path_ ,"Текстовый документ (*.txt);");
    }
    QVector<DataSeriesSensor> result;
    if(path_doc.isEmpty()){
        return result;
    }
    int count_now = 1;
    const int count_file = path_doc.size();
    for(const QString& path : path_doc){
        if(path.endsWith(".txt", Qt::CaseInsensitive)){
            QFileInfo file_info(path);
            QString name = file_info.baseName();
            DataSeriesSensor data;
            data = dow_file_.LoadDocAMT(path, name, count_file, count_now);
            result.push_back(data);
            save_path_ = file_info.absolutePath();
        }
    }
    return result;
}

void DocumentLoader::LoadEtalonFile(const QString& path_doc){
    if(path_doc.endsWith(".sml2", Qt::CaseInsensitive)){
        dow_file_.LoadDocEtalon_2v(path_doc);
        chart_view_->PanelLegendEtalon();
    }
    if(path_doc.endsWith(".sml", Qt::CaseInsensitive)){
        dow_file_.LoadDocEtalon(path_doc);
        chart_view_->PanelLegendEtalon();
    }
    if(path_doc.endsWith(".txt", Qt::CaseInsensitive)){
        dow_file_.LoadTXTEtalon(path_doc);
        chart_view_->PanelLegendEtalon();
    }
    if(path_doc.endsWith(".smv", Qt::CaseInsensitive)){
        const int first = all_sensors_.size();
        dow_file_.LoadSVDoc(path_doc, all_sensors_);
        chart_view_->PanelLegendEtalon();
        // Тот же порядок, что и при добавлении прибора вручную: сперва легенда
        // (она достраивает виджеты выбранных каналов), затем копия в модель —
        // обе копии должны делить одни и те же серии и метки.
        for(int i = first;i < all_sensors_.size();++i){
            chart_view_->PanelLegendACM(all_sensors_[i]);
            data_base_.AddDataSerACM(all_sensors_[i]);
        }
    }
}

bool DocumentLoader::OpenDocument(){
    QString path = save_path_.isEmpty() ? QApplication::applicationDirPath() : save_path_;
    QString path_doc = QFileDialog::getOpenFileName(dialog_parent_, "Открыть документ", path, "Формат SmartView (*.smv)");
    if(path_doc.isEmpty()){
        return false;
    }
    QFileInfo file_info(path_doc);
    save_path_ = file_info.absolutePath();
    first_open_doc_ = true;
    LoadEtalonFile(path_doc);
    chart_view_->ZoomOn();
    return true;
}

bool DocumentLoader::LoadEtalonInitial(){
    QString path = QApplication::applicationDirPath();
    QString path_doc = QFileDialog::getOpenFileName(dialog_parent_, "Открытие файла", path ,"(*.sml2);;(*.sml);;(*.txt);;(*.smv)");
    QFileInfo file_info(path_doc);
    save_path_ = file_info.absolutePath();
    first_open_doc_ = true;
    if(path_doc.isEmpty()){
        return false;
    }
    LoadEtalonFile(path_doc);
    chart_view_->ZoomOn();
    return true;
}

bool DocumentLoader::LoadEtalonReplace(){
    QString path = QApplication::applicationDirPath();
    QString path_doc = QFileDialog::getOpenFileName(dialog_parent_, "Открытие файла", path ,"(*.sml2);;(*.sml);;(*.smv)");
    if(path_doc.isEmpty()){
        return false;
    }
    LoadEtalonFile(path_doc);
    return true;
}
