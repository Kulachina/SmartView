#ifndef DOCUMENTLOADER_H
#define DOCUMENTLOADER_H
#pragma once
#include <QObject>
#include "Data.h"

class DataBase;
class DowlandFile;
class Las;
class ChartView;
class QWidget;

// Загрузка документов: диалоги открытия, диспетчеризация по расширению и парсинг.
// Возвращает распарсенные приборы; эталон грузит напрямую в DowlandFile/ChartView.
class DocumentLoader : public QObject {
    Q_OBJECT
public:
    DocumentLoader(DataBase& data_base,
                   DowlandFile& dow_file,
                   Las& las,
                   ChartView* chart_view,
                   QWidget* dialog_parent);
    QVector<DataSeriesSensor> LoadACM();
    QVector<DataSeriesSensor> LoadAMT();
    QVector<DataSeriesSensor> LoadLAS();
    bool LoadEtalonInitial();   // первичная загрузка эталона (с .txt, + ZoomOn)
    bool LoadEtalonReplace();   // загрузка нового эталона взамен текущего

private:
    void LoadEtalonFile(const QString& path);
    DataBase& data_base_;
    DowlandFile& dow_file_;
    Las& las_;
    ChartView* chart_view_;
    QWidget* dialog_parent_;
    QString save_path_;
    bool first_open_doc_ = false;
};

#endif // DOCUMENTLOADER_H
