#ifndef CANALUTILS_H
#define CANALUTILS_H
#pragma once
#include "Data.h"

// Удаление виджетов одного канала (серия, ось, метки, чекбокс, layout, модель).
void DeleteCanal(Canal& canal);

// Удаление прибора: все его каналы + метка прибора.
void DeleteSens(DataSeriesSensor& data);

#endif // CANALUTILS_H
