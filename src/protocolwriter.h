#ifndef PROTOCOLWRITER_H
#define PROTOCOLWRITER_H
#pragma once
// Запись протокола калибровки в .xlsx на основе встроенного шаблона
// (:/protocol_template.xlsx) через QXlsx. Здесь реализованы открытие шаблона
// и сохранение файла; вписывание ячеек из data — отдельно (помеченный блок в .cpp).
#include "protocoldata.h"

class QWidget;

class ProtocolWriter {
public:
    // Грузит шаблон, даёт вписать ячейки из data, спрашивает путь и сохраняет.
    // parent — родитель диалогов. Возвращает true при успешном сохранении,
    // false при ошибке или отмене пользователем.
    static bool Generate(const Protocol::Data& data, QWidget* parent = nullptr);
};

#endif // PROTOCOLWRITER_H
