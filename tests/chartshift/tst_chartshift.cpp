// Сдвиг кривых мышью: проверяем, что сдвиг попадает не только в QLineSeries,
// но и в points_rectangle/points_triangle канала — из них серия
// перезаполняется при переключении вида и из них же пишется документ .smv.
#include <QtTest>
#include <QApplication>
#include <QChart>
#include <QValueAxis>
#include <QLineSeries>
#include <QLabel>

#include "chartview.h"
#include "data_base.h"

namespace {

const qint64 kT0 = QDateTime(QDate(2026, 3, 2), QTime(10, 0, 0)).toMSecsSinceEpoch();
const int kRows = 200;

Canal MakeCanal(const QString& sensor, const QString& name){
    Canal c;
    c.name_canal = name;
    c.first_name_canal = name;
    c.name_sensor = sensor;
    c.name_unit = "кгс/см2";
    c.color_series_RGB = "255,0,0";
    c.select_box = true;
    c.unit_min = 0;
    c.unit_max = 100;
    for(int i = 0; i < kRows; ++i){
        const qint64 t = kT0 + i * 1000;
        const double y = 20.0 + (i % 50);
        // Ступенчатое представление: две точки на отсчёт, как у парсера.
        if(i > 0){
            c.points_rectangle.push_back(QPointF(t, c.points_rectangle.back().y()));
        }
        c.points_rectangle.push_back(QPointF(t, y));
        c.points_triangle.push_back(QPointF(t, y));
    }
    c.label = new QLabel();
    c.label_data = new QLabel();
    c.label_delta = new QLabel();
    c.label_name_canal = new QLabel(name);
    c.label_name_sensor = new QLabel(sensor);
    c.axis_y_ = new QValueAxis();
    c.axis_y_->setTickCount(21);
    c.series = new QLineSeries();
    c.series->setName(sensor + name);
    return c;
}

} // namespace

class TstChartShift : public QObject {
    Q_OBJECT
private slots:
    void shiftReachesCanalPoints();
};

void TstChartShift::shiftReachesCanalPoints(){
    DataBase db;
    ChartView view(nullptr, db);
    view.resize(900, 600);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    DataSeriesSensor sensor;
    sensor.name_sensor = "АЦМ-1";
    sensor.label_sensor = new QLabel(sensor.name_sensor);
    sensor.vec_canal.push_back(MakeCanal("АЦМ-1", "Давление"));
    sensor.vec_canal.push_back(MakeCanal("АЦМ-1", "Температура"));
    view.PanelLegendACM(sensor);
    db.AddDataSerACM(sensor);
    QCOMPARE(db.GetDataSerACM().size(), 1);
    QCOMPARE(db.GetDataSerACM()[0].vec_canal.size(), 2);

    view.GetAxisX()->setRange(QDateTime::fromMSecsSinceEpoch(kT0),
                              QDateTime::fromMSecsSinceEpoch(kT0 + kRows * 1000));
    QApplication::processEvents();

    Canal& canal = db.GetDataSerACM()[0].vec_canal[0];
    Canal& sibling = db.GetDataSerACM()[0].vec_canal[1];
    QLineSeries* series = canal.series;
    QVERIFY(series != nullptr);
    QCOMPARE(series->count(), canal.points_rectangle.size());

    const QVector<QPointF> rect_before = canal.points_rectangle;
    const QVector<QPointF> tri_before = canal.points_triangle;
    const QVector<QPointF> sibling_rect_before = sibling.points_rectangle;

    // Целимся точно в одну из точек кривой — так же, как это делает
    // ChartView::mousePressEvent при поиске захваченной серии.
    const QPointF grabbed = series->at(50);
    const QPoint from = view.GetChart()->mapToPosition(grabbed, series).toPoint();
    const QPoint mid = from + QPoint(40, 0);
    const QPoint to = from + QPoint(70, 0);

    const double dx_expected =
        view.GetChart()->mapToValue(to, series).x() - view.GetChart()->mapToValue(from, series).x();
    QVERIFY2(dx_expected > 1000.0, "сдвиг должен быть заметным по времени");

    view.ToogledFlagShiftSeries();   // включить режим «Сдвиг кривых»
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, from);
    QTest::mouseMove(view.viewport(), mid);
    QTest::mouseMove(view.viewport(), to);
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, to);

    // Серия сдвинулась — это работало и раньше.
    QCOMPARE(series->count(), rect_before.size());
    QVERIFY2(qAbs(series->at(0).x() - (rect_before[0].x() + dx_expected)) < 2.0,
             "серия не сдвинулась на ожидаемую величину");

    // Точки канала тоже — это и чинилось.
    QCOMPARE(canal.points_rectangle.size(), rect_before.size());
    QCOMPARE(canal.points_triangle.size(), tri_before.size());
    for(int i = 0; i < rect_before.size(); ++i){
        QVERIFY2(qAbs(canal.points_rectangle[i].x() - (rect_before[i].x() + dx_expected)) < 2.0,
                 "points_rectangle не сдвинулись");
        QCOMPARE(canal.points_rectangle[i].y(), rect_before[i].y());
    }
    for(int i = 0; i < tri_before.size(); ++i){
        QVERIFY2(qAbs(canal.points_triangle[i].x() - (tri_before[i].x() + dx_expected)) < 2.0,
                 "points_triangle не сдвинулись");
    }

    // Соседний канал того же прибора тянется вместе с захваченным.
    for(int i = 0; i < sibling_rect_before.size(); ++i){
        QVERIFY2(qAbs(sibling.points_rectangle[i].x() - (sibling_rect_before[i].x() + dx_expected)) < 2.0,
                 "второй канал прибора не сдвинулся");
    }

    // Ради чего всё: переключение вида (окно «Вид» перезаполняет серию из
    // векторов канала) больше не откатывает сдвиг.
    series->replace(canal.points_triangle);
    QVERIFY2(qAbs(series->at(0).x() - (tri_before[0].x() + dx_expected)) < 2.0,
             "сдвиг потерялся при переключении вида");
    series->replace(canal.points_rectangle);
    QVERIFY2(qAbs(series->at(0).x() - (rect_before[0].x() + dx_expected)) < 2.0,
             "сдвиг потерялся при возврате вида");

    // Второе перетаскивание складывается с первым, а не заменяет его.
    const QPoint from2 = view.GetChart()->mapToPosition(series->at(50), series).toPoint();
    const QPoint to2 = from2 + QPoint(30, 0);
    const double dx2 =
        view.GetChart()->mapToValue(to2, series).x() - view.GetChart()->mapToValue(from2, series).x();
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, from2);
    QTest::mouseMove(view.viewport(), to2);
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, to2);
    QVERIFY2(qAbs(canal.points_rectangle[0].x() - (rect_before[0].x() + dx_expected + dx2)) < 3.0,
             "второй сдвиг не накопился");
}

QTEST_MAIN(TstChartShift)
#include "tst_chartshift.moc"
