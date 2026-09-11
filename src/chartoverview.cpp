#include "chartoverview.h"
#include "data_base.h"
#include <QDateTimeAxis>
#include <QLineSeries>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <algorithm>

namespace {
// Минимальная ширина окна просмотра — иначе рамку невозможно было бы схватить.
constexpr qint64 kMinSpanMs = 1000;
// Кривая прореживается до такого числа столбцов; на каждый столбец остаётся
// минимум и максимум, поэтому силуэт (пики) сохраняется полностью.
constexpr int kBuckets = 1200;
// Полуширина зоны захвата края рамки, px.
constexpr int kEdgeGrab = 4;
constexpr int kMargin = 2;
}

ChartOverview::ChartOverview(DataBase& data_base, QDateTimeAxis* axis_time, QWidget* parent)
    : QWidget(parent), data_base_(data_base), axis_time_(axis_time){
    setMouseTracking(true);
    setMinimumHeight(80);
    setMaximumHeight(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setToolTip("Карта графика: тащите рамку — сдвиг, края рамки — масштаб,\n"
               "колесо — зум, двойной щелчок — вся запись");
}

QSize ChartOverview::sizeHint() const {
    return QSize(600, 90);
}

void ChartOverview::Clear(){
    curves_.clear();
    t_min_ = t_max_ = 0;
    win_min_ = win_max_ = 0;
    has_cursor_ = false;
    update();
}

void ChartOverview::Rebuild(){
    curves_.clear();
    has_cursor_ = false;
    QVector<DataSeriesEtalon>& etalon = data_base_.GetDataSerEtalon();
    for(DataSeriesEtalon& data : etalon){
        if(!data.series){
            continue;
        }
        AddCurve(data.series->points(), data.series->color());
    }
    // Полный диапазон карты — вся запись: объединение точек эталона и
    // диапазона оси по умолчанию (он же используется при откате зума).
    bool first = true;
    for(const Curve& c : curves_){
        for(const QPointF& p : c.points){
            const qint64 x = static_cast<qint64>(p.x());
            if(first){
                t_min_ = t_max_ = x;
                first = false;
            } else {
                t_min_ = qMin(t_min_, x);
                t_max_ = qMax(t_max_, x);
            }
        }
    }
    const std::pair<QDateTime,QDateTime> def = data_base_.GetDefaultAxisX();
    if(def.first.isValid() && def.second.isValid()){
        const qint64 lo = def.first.toMSecsSinceEpoch();
        const qint64 hi = def.second.toMSecsSinceEpoch();
        if(first){
            t_min_ = lo;
            t_max_ = hi;
        } else {
            t_min_ = qMin(t_min_, lo);
            t_max_ = qMax(t_max_, hi);
        }
    }
    if(axis_time_){
        win_min_ = axis_time_->min().toMSecsSinceEpoch();
        win_max_ = axis_time_->max().toMSecsSinceEpoch();
    }
    update();
}

void ChartOverview::AddCurve(const QVector<QPointF>& src, const QColor& color){
    if(src.isEmpty()){
        return;
    }
    Curve curve;
    curve.color = color.isValid() ? color : QColor(Qt::darkGray);
    double x_min = src.first().x(),
           x_max = src.first().x();
    curve.y_min = src.first().y();
    curve.y_max = src.first().y();
    for(const QPointF& p : src){
        x_min = qMin(x_min, p.x());
        x_max = qMax(x_max, p.x());
        curve.y_min = qMin(curve.y_min, p.y());
        curve.y_max = qMax(curve.y_max, p.y());
    }
    if(src.size() <= kBuckets * 2 || x_max <= x_min){
        curve.points = src;
    } else {
        // Прореживание по столбцам: по два экстремума на столбец в порядке
        // появления. Полная кривая эталона — десятки тысяч точек, и
        // перерисовывать её на каждое движение мыши нельзя.
        const double span = x_max - x_min;
        QVector<QPointF> low(kBuckets), high(kBuckets);
        QVector<bool> used(kBuckets, false);
        for(const QPointF& p : src){
            int idx = static_cast<int>((p.x() - x_min) / span * kBuckets);
            idx = qBound(0, idx, kBuckets - 1);
            if(!used[idx]){
                used[idx] = true;
                low[idx] = high[idx] = p;
            } else {
                if(p.y() < low[idx].y()){ low[idx] = p; }
                if(p.y() > high[idx].y()){ high[idx] = p; }
            }
        }
        curve.points.reserve(kBuckets * 2);
        for(int i = 0; i < kBuckets; ++i){
            if(!used[i]){
                continue;
            }
            if(low[i].x() <= high[i].x()){
                curve.points.push_back(low[i]);
                curve.points.push_back(high[i]);
            } else {
                curve.points.push_back(high[i]);
                curve.points.push_back(low[i]);
            }
        }
    }
    if(qFuzzyCompare(curve.y_min, curve.y_max)){
        curve.y_min -= 1;
        curve.y_max += 1;
    }
    curves_.push_back(curve);
}

QRectF ChartOverview::PlotRect() const {
    return QRectF(rect()).adjusted(kMargin, kMargin, -kMargin - 1, -kMargin - 1);
}

double ChartOverview::MsToX(qint64 ms) const {
    const QRectF plot = PlotRect();
    if(t_max_ <= t_min_){
        return plot.left();
    }
    const double k = static_cast<double>(ms - t_min_) / static_cast<double>(t_max_ - t_min_);
    return plot.left() + k * plot.width();
}

qint64 ChartOverview::XToMs(double x) const {
    const QRectF plot = PlotRect();
    if(plot.width() <= 0){
        return t_min_;
    }
    const double k = (x - plot.left()) / plot.width();
    return t_min_ + static_cast<qint64>(k * static_cast<double>(t_max_ - t_min_));
}

void ChartOverview::paintEvent(QPaintEvent*){
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF plot = PlotRect();
    painter.fillRect(rect(), Qt::white);
    if(!HasData()){
        painter.setPen(QColor(160, 160, 160));
        painter.drawRect(plot);
        return;
    }
    // Сетка.
    painter.setPen(QPen(QColor(215, 215, 215), 1, Qt::DotLine));
    for(int i = 1; i < 10; ++i){
        const double x = plot.left() + plot.width() * i / 10.0;
        painter.drawLine(QPointF(x, plot.top()), QPointF(x, plot.bottom()));
    }
    for(int i = 1; i < 4; ++i){
        const double y = plot.top() + plot.height() * i / 4.0;
        painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
    }
    // Кривые: каждая нормирована по своему диапазону значений, как и на
    // основном графике с отдельными осями Y.
    painter.setClipRect(plot);
    for(const Curve& curve : curves_){
        const double span = curve.y_max - curve.y_min;
        const double pad = span * 0.08;
        const double lo = curve.y_min - pad;
        const double hi = curve.y_max + pad;
        QPainterPath path;
        bool started = false;
        for(const QPointF& p : curve.points){
            const double x = MsToX(static_cast<qint64>(p.x()));
            const double k = (p.y() - lo) / (hi - lo);
            const double y = plot.bottom() - k * plot.height();
            if(!started){
                path.moveTo(x, y);
                started = true;
            } else {
                path.lineTo(x, y);
            }
        }
        painter.setPen(QPen(curve.color, 1));
        painter.drawPath(path);
    }
    painter.setClipping(false);
    // Окно просмотра: затемняем всё, что вне него, и обводим рамкой.
    double x1 = MsToX(win_min_),
           x2 = MsToX(win_max_);
    if(x1 > x2){
        std::swap(x1, x2);
    }
    x1 = qBound(plot.left(), x1, plot.right());
    x2 = qBound(plot.left(), x2, plot.right());
    const QColor dim(120, 120, 120, 45);
    if(x1 > plot.left()){
        painter.fillRect(QRectF(plot.left(), plot.top(), x1 - plot.left(), plot.height()), dim);
    }
    if(x2 < plot.right()){
        painter.fillRect(QRectF(x2, plot.top(), plot.right() - x2, plot.height()), dim);
    }
    const QRectF window(QPointF(x1, plot.top()), QPointF(x2, plot.bottom()));
    painter.setPen(QPen(QColor(40, 90, 170), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(window);
    // Рукоятки краёв.
    painter.setBrush(QColor(40, 90, 170));
    painter.setPen(Qt::NoPen);
    const double handle_h = qMin(14.0, plot.height() / 2);
    const double handle_y = plot.center().y() - handle_h / 2;
    painter.drawRect(QRectF(x1 - 1, handle_y, 3, handle_h));
    painter.drawRect(QRectF(x2 - 1, handle_y, 3, handle_h));
    // Курсор основного графика.
    if(has_cursor_ && cursor_ms_ >= t_min_ && cursor_ms_ <= t_max_){
        const double cx = MsToX(cursor_ms_);
        painter.setPen(QPen(QColor(0, 0, 255), 1));
        painter.drawLine(QPointF(cx, plot.top()), QPointF(cx, plot.bottom()));
    }
    painter.setPen(QColor(120, 120, 120));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(plot);
}

ChartOverview::Grab ChartOverview::HitTest(int x) const {
    double x1 = MsToX(win_min_),
           x2 = MsToX(win_max_);
    if(x1 > x2){
        std::swap(x1, x2);
    }
    if(qAbs(x - x1) <= kEdgeGrab){
        return Grab::Left;
    }
    if(qAbs(x - x2) <= kEdgeGrab){
        return Grab::Right;
    }
    if(x > x1 && x < x2){
        return Grab::Move;
    }
    return Grab::None;
}

void ChartOverview::ApplyRange(qint64 min_ms, qint64 max_ms){
    if(!axis_time_ || !HasData()){
        return;
    }
    if(max_ms - min_ms < kMinSpanMs){
        return;
    }
    if(!gesture_saved_){
        emit ZoomAboutToChange();
        gesture_saved_ = true;
    }
    win_min_ = min_ms;
    win_max_ = max_ms;
    syncing_ = true;
    axis_time_->setRange(QDateTime::fromMSecsSinceEpoch(min_ms),
                         QDateTime::fromMSecsSinceEpoch(max_ms));
    syncing_ = false;
    update();
}

void ChartOverview::OnViewRangeChanged(QDateTime min, QDateTime max){
    if(syncing_){
        return;
    }
    win_min_ = min.toMSecsSinceEpoch();
    win_max_ = max.toMSecsSinceEpoch();
    update();
}

void ChartOverview::SetCursorTime(qint64 ms){
    cursor_ms_ = ms;
    has_cursor_ = true;
    update();
}

void ChartOverview::ClearCursorTime(){
    if(!has_cursor_){
        return;
    }
    has_cursor_ = false;
    update();
}

void ChartOverview::mousePressEvent(QMouseEvent* event){
    if(!HasData() || event->button() != Qt::LeftButton){
        QWidget::mousePressEvent(event);
        return;
    }
    const int x = static_cast<int>(event->position().x());
    grab_ = HitTest(x);
    gesture_saved_ = false;
    if(grab_ == Grab::None){
        // Щелчок мимо рамки — переносим окно центром в точку щелчка.
        const qint64 span = win_max_ - win_min_;
        qint64 min_ms = XToMs(x) - span / 2;
        min_ms = qBound(t_min_, min_ms, qMax(t_min_, t_max_ - span));
        ApplyRange(min_ms, min_ms + span);
        grab_ = Grab::Move;
    }
    drag_offset_ = static_cast<qint64>(x - MsToX(win_min_));
    setCursor(grab_ == Grab::Move ? Qt::ClosedHandCursor : Qt::SizeHorCursor);
}

void ChartOverview::mouseMoveEvent(QMouseEvent* event){
    if(!HasData()){
        QWidget::mouseMoveEvent(event);
        return;
    }
    const int x = static_cast<int>(event->position().x());
    if(grab_ == Grab::None){
        const Grab hover = HitTest(x);
        if(hover == Grab::Left || hover == Grab::Right){
            setCursor(Qt::SizeHorCursor);
        } else if(hover == Grab::Move){
            setCursor(Qt::OpenHandCursor);
        } else {
            unsetCursor();
        }
        QWidget::mouseMoveEvent(event);
        return;
    }
    if(grab_ == Grab::Move){
        const qint64 span = win_max_ - win_min_;
        qint64 min_ms = XToMs(x - drag_offset_);
        min_ms = qBound(t_min_, min_ms, qMax(t_min_, t_max_ - span));
        ApplyRange(min_ms, min_ms + span);
    } else if(grab_ == Grab::Left){
        const qint64 min_ms = qBound(t_min_, XToMs(x), win_max_ - kMinSpanMs);
        ApplyRange(min_ms, win_max_);
    } else if(grab_ == Grab::Right){
        const qint64 max_ms = qBound(win_min_ + kMinSpanMs, XToMs(x), t_max_);
        ApplyRange(win_min_, max_ms);
    }
}

void ChartOverview::mouseReleaseEvent(QMouseEvent* event){
    grab_ = Grab::None;
    gesture_saved_ = false;
    unsetCursor();
    QWidget::mouseReleaseEvent(event);
}

void ChartOverview::mouseDoubleClickEvent(QMouseEvent* event){
    if(!HasData() || event->button() != Qt::LeftButton){
        QWidget::mouseDoubleClickEvent(event);
        return;
    }
    gesture_saved_ = false;
    ApplyRange(t_min_, t_max_);
    gesture_saved_ = false;
}

void ChartOverview::wheelEvent(QWheelEvent* event){
    if(!HasData()){
        QWidget::wheelEvent(event);
        return;
    }
    const int steps = event->angleDelta().y();
    if(steps == 0){
        QWidget::wheelEvent(event);
        return;
    }
    const double factor = steps > 0 ? 1.0 / 1.25 : 1.25;
    const qint64 anchor = XToMs(event->position().x());
    qint64 left = anchor - static_cast<qint64>((anchor - win_min_) * factor);
    qint64 right = anchor + static_cast<qint64>((win_max_ - anchor) * factor);
    left = qMax(t_min_, left);
    right = qMin(t_max_, right);
    if(right - left < kMinSpanMs){
        return;
    }
    gesture_saved_ = false;
    ApplyRange(left, right);
    gesture_saved_ = false;
    event->accept();
}

void ChartOverview::leaveEvent(QEvent* event){
    if(grab_ == Grab::None){
        unsetCursor();
    }
    QWidget::leaveEvent(event);
}
