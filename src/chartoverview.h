#ifndef CHARTOVERVIEW_H
#define CHARTOVERVIEW_H
#pragma once
#include <QWidget>
#include <QColor>
#include <QDateTime>
#include <QPointF>
#include <QVector>

class DataBase;
class QDateTimeAxis;

// Карта графика: сжатый обзор всей записи эталона с рамкой текущего окна
// просмотра. Перетаскивание рамки двигает ось времени основного графика,
// потягивание за края — меняет её масштаб. Обратная синхронизация идёт через
// сигнал QDateTimeAxis::rangeChanged, поэтому любой зум основного графика
// (выделение рамкой, откат правой кнопкой, загрузка документа) карта
// подхватывает сама.
class ChartOverview : public QWidget {
    Q_OBJECT
public:
    ChartOverview(DataBase& data_base, QDateTimeAxis* axis_time, QWidget* parent = nullptr);
    void Rebuild();   // пересобрать кривые карты по текущим данным эталона
    void Clear();     // документ закрыт — карта пустая
    QSize sizeHint() const override;
signals:
    // Перед первым изменением диапазона в жесте: основной график сохраняет
    // состояние зума, чтобы откат правой кнопкой работал и после карты.
    void ZoomAboutToChange();
public slots:
    void OnViewRangeChanged(QDateTime min, QDateTime max);
    void SetCursorTime(qint64 ms);   // положение курсора на основном графике
    void ClearCursorTime();
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void leaveEvent(QEvent* event) override;
private:
    enum class Grab { None, Move, Left, Right };
    struct Curve {
        QVector<QPointF> points;   // прорежённая кривая: x — мс, y — значение
        double y_min = 0,
               y_max = 0;
        QColor color;
    };
    QRectF PlotRect() const;
    double MsToX(qint64 ms) const;
    qint64 XToMs(double x) const;
    Grab HitTest(int x) const;
    void ApplyRange(qint64 min_ms, qint64 max_ms);
    void AddCurve(const QVector<QPointF>& src, const QColor& color);
    bool HasData() const { return !curves_.isEmpty() && t_max_ > t_min_; }
    DataBase& data_base_;
    QDateTimeAxis* axis_time_;
    QVector<Curve> curves_;
    qint64 t_min_ = 0,          // полный диапазон записи
           t_max_ = 0,
           win_min_ = 0,        // текущее окно просмотра
           win_max_ = 0,
           cursor_ms_ = 0,
           drag_offset_ = 0;    // смещение точки захвата от левого края окна
    Grab grab_ = Grab::None;
    bool syncing_ = false,      // защита от рекурсии карта -> ось -> карта
         gesture_saved_ = false,
         has_cursor_ = false;
};

#endif // CHARTOVERVIEW_H
