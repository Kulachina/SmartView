#ifndef PROTOCOLDATA_H
#define PROTOCOLDATA_H
#pragma once
// Контракт данных для протокола калибровки. Метаданные заполняет форма,
// измерения (pressure_blocks/temperature_points) — код SmartView, погрешности
// и заключение считает ProtocolWriter. Число блоков и точек — переменное.
#include <QString>
#include <QStringList>
#include <QVector>

namespace Protocol {

// Тип погрешности канала (как в исходном Excel: Абс/Отн/Прив).
enum class ErrType { Absolute, Relative, Reduced };

// Одна точка: показание эталона и показание прибора.
struct Point {
    double reference = 0.0;   // показания эталона
    double device    = 0.0;   // показания прибора
};

// Блок измерений давления при фиксированной температуре.
struct PressureBlock {
    double temperature = 0.0;    // для заголовка "Измерения при N ℃"
    QVector<Point> points;       // точки давления (переменное число)
};

// Метрологические характеристики канала.
struct ChannelSpec {
    QString unit;                        // ед. изм. канала ("кгс/см²", "°С")
    ErrType error_type = ErrType::Reduced;
    double  error_limit = 0.0;           // предел допускаемой погрешности (±)
    QString error_unit;                  // ед. изм. погрешности ("%", "°С")
    double  span = 1.0;                  // диапазон для Прив: P=1000, T=120
};

// Полные данные протокола.
struct Data {
    // --- метаданные (форма) ---
    QString number;              // № протокола
    QString date;                // дата
    QString device_name;         // наименование аппаратуры
    QString device_type;         // тип аппаратуры
    QString serial;              // заводской номер
    QString customer;            // наименование и ИНН заказчика
    QString basis;               // калибровано на основании
    QString location;            // место проведения ("\n" — перенос на 2-ю строку)
    QString period;              // период калибровки
    QString ambient_temp;        // температура окружающей среды, °C
    QString humidity;            // относительная влажность воздуха, %
    QString atm_pressure;        // атмосферное давление, мм.рт.ст
    QString responsible;         // ФИО ответственного за калибровку
    QStringList calib_means;     // средства калибровки (по одной строке на пункт)
    QString external_inspection = "удовлетворительно";   // внешний осмотр
    QString trial               = "удовлетворительно";   // опробование

    // --- характеристики каналов ---
    ChannelSpec pressure_spec;
    ChannelSpec temperature_spec;

    // --- измерения (заполняет код SmartView) ---
    QVector<PressureBlock> pressure_blocks;    // переменное число блоков и точек
    QVector<Point>         temperature_points; // переменное число точек
};

// Погрешность одной точки по типу канала (совпадает с формулами Excel).
inline double CalcError(const ChannelSpec& s, const Point& p) {
    switch (s.error_type) {
        case ErrType::Absolute:
            return p.device - p.reference;
        case ErrType::Relative:
            return p.reference != 0.0 ? (p.device - p.reference) / p.reference * 100.0 : 0.0;
        case ErrType::Reduced:
            return s.span != 0.0 ? (p.device - p.reference) / s.span * 100.0 : 0.0;
    }
    return 0.0;
}

// true, если все точки обоих каналов в пределах допускаемой погрешности.
inline bool IsWithinLimits(const Data& d) {
    for (const PressureBlock& b : d.pressure_blocks)
        for (const Point& p : b.points)
            if (qAbs(CalcError(d.pressure_spec, p)) > d.pressure_spec.error_limit)
                return false;
    for (const Point& p : d.temperature_points)
        if (qAbs(CalcError(d.temperature_spec, p)) > d.temperature_spec.error_limit)
            return false;
    return true;
}

} // namespace Protocol

#endif // PROTOCOLDATA_H
