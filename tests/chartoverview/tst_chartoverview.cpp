// Карта графика: проверяем, что рамка окна просмотра ходит в обе стороны —
// перетаскивание по карте двигает ось времени основного графика, а изменение
// оси (зум рамкой, откат, загрузка документа) двигает рамку на карте.
#include <QtTest>
#include <QApplication>
#include <QDateTimeAxis>
#include <QLineSeries>
#include <QMouseEvent>
#include <QPixmap>
#include <QSignalSpy>

#include "chartoverview.h"
#include "data_base.h"

namespace {

const qint64 kT0 = QDateTime(QDate(2026, 3, 2), QTime(10, 0, 0)).toMSecsSinceEpoch();
const qint64 kSpan = 600000;   // вся запись — 10 минут
const int kPoints = 6000;
const int kWidth = 800;
const int kHeight = 90;

void FillEtalon(DataBase& db){
    for(int s = 0; s < 2; ++s){
        DataSeriesEtalon data;
        data.name_series = s == 0 ? "ЛТ300" : "ДМ5002М";
        data.series = new QLineSeries();
        data.series->setName(data.name_series);
        data.series->setColor(s == 0 ? QColor("blue") : QColor("red"));
        QVector<QPointF> points;
        points.reserve(kPoints);
        for(int i = 0; i < kPoints; ++i){
            const qint64 t = kT0 + kSpan * i / kPoints;
            points.push_back(QPointF(t, 50.0 + 10.0 * qSin(i / 100.0) + s * 100));
        }
        data.series->replace(points);
        data.points_rectangle_view = points;
        db.AddDataSerEtalon(data);
    }
    db.SetDefaultAxisX(QDateTime::fromMSecsSinceEpoch(kT0 + kSpan),
                       QDateTime::fromMSecsSinceEpoch(kT0));
}

void SendMouse(QWidget* w, QEvent::Type type, int x, Qt::MouseButton button, Qt::MouseButtons buttons){
    const QPointF pos(x, kHeight / 2);
    QMouseEvent event(type, pos, w->mapToGlobal(pos), button, buttons, Qt::NoModifier);
    QApplication::sendEvent(w, &event);
}

} // namespace

class TstChartOverview : public QObject {
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void rebuildTakesWholeRecord();
    void axisRangeMovesFrame();
    void dragFrameMovesAxis();
    void dragEdgeChangesSpan();
    void doubleClickRestoresWholeRecord();
    void oneZoomStatePerGesture();
    void paintsWithoutData();
private:
    void Zoom(qint64 from, qint64 to);
    DataBase* db_ = nullptr;
    QDateTimeAxis* axis_ = nullptr;
    ChartOverview* map_ = nullptr;
};

void TstChartOverview::init(){
    db_ = new DataBase();
    FillEtalon(*db_);
    axis_ = new QDateTimeAxis();
    axis_->setRange(QDateTime::fromMSecsSinceEpoch(kT0),
                    QDateTime::fromMSecsSinceEpoch(kT0 + kSpan));
    map_ = new ChartOverview(*db_, axis_);
    QObject::connect(axis_, &QDateTimeAxis::rangeChanged,
                     map_, &ChartOverview::OnViewRangeChanged);
    map_->resize(kWidth, kHeight);
    map_->show();
    map_->Rebuild();
}

void TstChartOverview::cleanup(){
    delete map_;
    delete axis_;
    delete db_;
    map_ = nullptr;
    axis_ = nullptr;
    db_ = nullptr;
}

void TstChartOverview::Zoom(qint64 from, qint64 to){
    axis_->setRange(QDateTime::fromMSecsSinceEpoch(from),
                    QDateTime::fromMSecsSinceEpoch(to));
}

void TstChartOverview::rebuildTakesWholeRecord(){
    // Окно во всю запись: щелчок у любого края не должен ничего менять —
    // двигать некуда.
    const qint64 before_min = axis_->min().toMSecsSinceEpoch();
    SendMouse(map_, QEvent::MouseButtonPress, kWidth / 2, Qt::LeftButton, Qt::LeftButton);
    SendMouse(map_, QEvent::MouseMove, kWidth / 2 + 100, Qt::NoButton, Qt::LeftButton);
    SendMouse(map_, QEvent::MouseButtonRelease, kWidth / 2 + 100, Qt::LeftButton, Qt::NoButton);
    QCOMPARE(axis_->min().toMSecsSinceEpoch(), before_min);
    QCOMPARE(axis_->max().toMSecsSinceEpoch(), kT0 + kSpan);
}

void TstChartOverview::axisRangeMovesFrame(){
    // Зум основного графика -> рамка сузилась. Проверяем косвенно: середина
    // карты теперь вне рамки, и щелчок туда переносит окно (а не игнорируется).
    Zoom(kT0, kT0 + kSpan / 10);
    const qint64 span = axis_->max().toMSecsSinceEpoch() - axis_->min().toMSecsSinceEpoch();
    QCOMPARE(span, kSpan / 10);
    SendMouse(map_, QEvent::MouseButtonPress, kWidth / 2, Qt::LeftButton, Qt::LeftButton);
    SendMouse(map_, QEvent::MouseButtonRelease, kWidth / 2, Qt::LeftButton, Qt::NoButton);
    // Окно переехало центром примерно в середину записи.
    const qint64 mid = (axis_->min().toMSecsSinceEpoch() + axis_->max().toMSecsSinceEpoch()) / 2;
    QVERIFY2(qAbs(mid - (kT0 + kSpan / 2)) < kSpan / 50,
             qPrintable(QString("центр окна %1, ожидался ~%2").arg(mid).arg(kT0 + kSpan / 2)));
    // Ширина окна сохранилась.
    QCOMPARE(axis_->max().toMSecsSinceEpoch() - axis_->min().toMSecsSinceEpoch(), span);
}

void TstChartOverview::dragFrameMovesAxis(){
    Zoom(kT0 + kSpan / 4, kT0 + kSpan / 2);
    const qint64 span = axis_->max().toMSecsSinceEpoch() - axis_->min().toMSecsSinceEpoch();
    const qint64 before_min = axis_->min().toMSecsSinceEpoch();
    // Хватаем рамку за середину и тащим вправо на 40 px.
    const double plot_w = kWidth - 5;
    const int x_mid = static_cast<int>(5 + plot_w * ((before_min + span / 2 - kT0) / double(kSpan)));
    const int dx = 40;
    SendMouse(map_, QEvent::MouseButtonPress, x_mid, Qt::LeftButton, Qt::LeftButton);
    SendMouse(map_, QEvent::MouseMove, x_mid + dx, Qt::NoButton, Qt::LeftButton);
    SendMouse(map_, QEvent::MouseButtonRelease, x_mid + dx, Qt::LeftButton, Qt::NoButton);
    const qint64 after_min = axis_->min().toMSecsSinceEpoch();
    const qint64 expected = before_min + static_cast<qint64>(dx / plot_w * kSpan);
    const qint64 tolerance = static_cast<qint64>(3 * kSpan / plot_w);
    QVERIFY2(qAbs(after_min - expected) <= tolerance,
             qPrintable(QString("левый край %1, ожидался ~%2").arg(after_min).arg(expected)));
    QCOMPARE(axis_->max().toMSecsSinceEpoch() - after_min, span);
}

void TstChartOverview::dragEdgeChangesSpan(){
    Zoom(kT0 + kSpan / 4, kT0 + kSpan / 2);
    const qint64 before_min = axis_->min().toMSecsSinceEpoch();
    const qint64 before_max = axis_->max().toMSecsSinceEpoch();
    const double plot_w = kWidth - 5;
    const int x_right = static_cast<int>(5 + plot_w * ((before_max - kT0) / double(kSpan)));
    // Тянем правый край рамки внутрь — окно должно сузиться, левый край стоять.
    SendMouse(map_, QEvent::MouseButtonPress, x_right, Qt::LeftButton, Qt::LeftButton);
    SendMouse(map_, QEvent::MouseMove, x_right - 60, Qt::NoButton, Qt::LeftButton);
    SendMouse(map_, QEvent::MouseButtonRelease, x_right - 60, Qt::LeftButton, Qt::NoButton);
    QCOMPARE(axis_->min().toMSecsSinceEpoch(), before_min);
    QVERIFY2(axis_->max().toMSecsSinceEpoch() < before_max,
             "правый край рамки не сузил окно");
    QVERIFY(axis_->max().toMSecsSinceEpoch() > before_min);
}

void TstChartOverview::doubleClickRestoresWholeRecord(){
    Zoom(kT0 + kSpan / 4, kT0 + kSpan / 2);
    const QPointF pos(kWidth / 2, kHeight / 2);
    QMouseEvent event(QEvent::MouseButtonDblClick, pos, map_->mapToGlobal(pos),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(map_, &event);
    QCOMPARE(axis_->min().toMSecsSinceEpoch(), kT0);
    QCOMPARE(axis_->max().toMSecsSinceEpoch(), kT0 + kSpan);
}

void TstChartOverview::oneZoomStatePerGesture(){
    // Стек отката не должен пухнуть: за одно перетаскивание — одно сохранение.
    Zoom(kT0 + kSpan / 4, kT0 + kSpan / 2);
    QSignalSpy spy(map_, &ChartOverview::ZoomAboutToChange);
    const double plot_w = kWidth - 5;
    const int x_mid = static_cast<int>(5 + plot_w * 0.375);
    SendMouse(map_, QEvent::MouseButtonPress, x_mid, Qt::LeftButton, Qt::LeftButton);
    for(int i = 1; i <= 20; ++i){
        SendMouse(map_, QEvent::MouseMove, x_mid + i, Qt::NoButton, Qt::LeftButton);
    }
    SendMouse(map_, QEvent::MouseButtonRelease, x_mid + 20, Qt::LeftButton, Qt::NoButton);
    QCOMPARE(spy.count(), 1);
}

void TstChartOverview::paintsWithoutData(){
    // Пустой документ: карта рисуется и не реагирует на мышь, без падений.
    map_->Clear();
    QPixmap pixmap(kWidth, kHeight);
    map_->render(&pixmap);
    const qint64 before = axis_->min().toMSecsSinceEpoch();
    SendMouse(map_, QEvent::MouseButtonPress, kWidth / 2, Qt::LeftButton, Qt::LeftButton);
    SendMouse(map_, QEvent::MouseMove, kWidth / 2 + 50, Qt::NoButton, Qt::LeftButton);
    SendMouse(map_, QEvent::MouseButtonRelease, kWidth / 2 + 50, Qt::LeftButton, Qt::NoButton);
    QCOMPARE(axis_->min().toMSecsSinceEpoch(), before);
}

QTEST_MAIN(TstChartOverview)
#include "tst_chartoverview.moc"
