#ifndef DELETESENSORWINDOW_H
#define DELETESENSORWINDOW_H
#pragma once
#include <QWidget>
#include "Data.h"

class DataBase;
class QComboBox;

// Окно «Удалить прибор»: выбор прибора из списка и его удаление.
class DeleteSensorWindow : public QWidget {
    Q_OBJECT
public:
    DeleteSensorWindow(DataBase& data_base, QVector<DataSeriesSensor>& all_data_sensor, QWidget* parent = nullptr);
    void Refresh();   // обновить список приборов перед показом

private:
    void DeleteSelected();
    DataBase& data_base_;
    QVector<DataSeriesSensor>& all_data_sensor_;
    QComboBox* combo_del_sens_;
};

#endif // DELETESENSORWINDOW_H
