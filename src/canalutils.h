#ifndef CANALUTILS_H
#define CANALUTILS_H
#pragma once
#include "Data.h"

// Удаление виджетов одного канала (серия, ось, метки, чекбокс, layout, модель).
void DeleteCanal(Canal& canal);

// Удаление прибора: все его каналы + метка прибора.
void DeleteSens(DataSeriesSensor& data);

// Полное состояние приборов для записи в документ.
// all_sensors — все приборы со всеми каналами, включая невыбранные; их копии
// каналов не обновляются во время работы. model — рабочая модель, где живут
// актуальные настройки и рассчитанные check_points/delta_points, но только по
// выбранным каналам. Для каждого канала берётся версия из model, если она там
// есть; приборы, которых нет в all_sensors, добавляются целиком.
QVector<DataSeriesSensor> SnapshotSensors(const QVector<DataSeriesSensor>& all_sensors,
                                          const QVector<DataSeriesSensor>& model);

#endif // CANALUTILS_H
