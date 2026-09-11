// Регрессионный тест формата документа .smv: сохранение → загрузка → сравнение.
// Проверяет, что документ переживает круг без потерь: эталон (кривые, условия,
// диапазоны осей, контрольные точки и диапазоны) и приборы со всеми
// настройками каналов.
#include <QtTest>
#include <QApplication>
#include <QTemporaryDir>
#include <QChart>
#include <QDateTimeAxis>
#include <QValueAxis>

#include "dowland_file.h"
#include "data_base.h"
#include "canalutils.h"

namespace {

const qint64 kT0 = QDateTime(QDate(2026, 3, 2), QTime(10, 0, 0)).toMSecsSinceEpoch();

DataSeriesEtalon MakeEtalon(const QString& name, double y_min, double y_max, double base){
    DataSeriesEtalon doc;
    doc.name_series = name;
    doc.series = new QLineSeries();
    doc.point_series = new QScatterSeries();
    doc.axis_y_ = new QValueAxis();
    doc.axis_y_->setRange(y_min, y_max);
    doc.data_sensor = new QLabel();
    doc.label_point = new QLabel();
    for(int i = 0; i < 5; ++i){
        doc.points_triangle_view.push_back(QPointF(kT0 + i * 1000, base + i));
        doc.points_rectangle_view.push_back(QPointF(kT0 + i * 1000, base + i * 0.5));
    }
    doc.condition = {21.5, 48.0, 750.4};
    return doc;
}

Canal MakeCanal(const QString& sensor, const QString& name, int type_error, bool selected){
    Canal c;
    c.name_canal = name;
    c.first_name_canal = name + "_orig";
    c.new_name_canal = name + "_new";
    c.name_sensor = sensor;
    c.name_unit = "кгс/см2";
    c.color_series_ = "Красный";
    c.color_series_RGB = "255,0,0";
    c.unit_min = -1.5;
    c.unit_max = 63.25;
    c.accept_min = -0.25;
    c.accept_max = 0.25;
    c.type_error = type_error;
    c.duration_error_min = 3;
    c.duration_error_max = 17;
    c.select_box = selected;
    c.check_ACP = true;
    c.first_unit = false;
    c.check_points = {1.0, 2.5, 3.75};
    c.delta_points = {0.01, -0.02, 0.03};
    for(int i = 0; i < 4; ++i){
        c.points_rectangle.push_back(QPointF(kT0 + i * 1000, i * 1.5));
        c.points_triangle.push_back(QPointF(kT0 + i * 1000, i * 2.5));
    }
    return c;
}

void CompareCanal(const Canal& a, const Canal& b){
    QCOMPARE(a.name_canal, b.name_canal);
    QCOMPARE(a.first_name_canal, b.first_name_canal);
    QCOMPARE(a.new_name_canal, b.new_name_canal);
    QCOMPARE(a.name_sensor, b.name_sensor);
    QCOMPARE(a.name_unit, b.name_unit);
    QCOMPARE(a.color_series_, b.color_series_);
    QCOMPARE(a.color_series_RGB, b.color_series_RGB);
    QCOMPARE(a.unit_min, b.unit_min);
    QCOMPARE(a.unit_max, b.unit_max);
    QCOMPARE(a.accept_min, b.accept_min);
    QCOMPARE(a.accept_max, b.accept_max);
    QCOMPARE(a.type_error, b.type_error);
    QCOMPARE(a.duration_error_min, b.duration_error_min);
    QCOMPARE(a.duration_error_max, b.duration_error_max);
    QCOMPARE(a.select_box, b.select_box);
    QCOMPARE(a.check_ACP, b.check_ACP);
    QCOMPARE(a.first_unit, b.first_unit);
    QCOMPARE(a.check_points, b.check_points);
    QCOMPARE(a.delta_points, b.delta_points);
    QCOMPARE(a.points_rectangle, b.points_rectangle);
    QCOMPARE(a.points_triangle, b.points_triangle);
}

} // namespace

class TstSvFormat : public QObject {
    Q_OBJECT
private slots:
    void roundTrip();
    void snapshotTakesRuntimeState();
};

void TstSvFormat::roundTrip(){
    QChart chart;
    QDateTimeAxis axis_x;
    QValueAxis axis_temp;
    QValueAxis axis_bar;
    chart.addAxis(&axis_x, Qt::AlignBottom);
    chart.addAxis(&axis_temp, Qt::AlignLeft);
    chart.addAxis(&axis_bar, Qt::AlignRight);

    DataBase db;
    DowlandFile dow(db);
    dow.SetChartDoc(&chart, &axis_temp, &axis_bar);
    dow.SetAxisTime(&axis_x);

    // --- исходный документ -------------------------------------------------
    db.GetDataSerEtalon().push_back(MakeEtalon("ЛТ300", 10.0, 90.5, 20.0));
    db.GetDataSerEtalon().push_back(MakeEtalon("ДМ5002М", 0.0, 64.75, 5.0));
    db.AddCheckPoint(kT0 + 1000, 21.0, 6.0);
    db.AddCheckPoint(kT0 + 3000, 23.0, 8.0);
    // КТ, добавленная пользователем через окно «Контрольные точки»: оно ведёт
    // только эту тройку и не трогает check_points64_.
    db.GetCheckPoints().push_back(QDateTime::fromMSecsSinceEpoch(kT0 + 3500));
    db.GetCheckPointTemp().push_back(23.5);
    db.GetCheckPointBar().push_back(8.5);
    db.GetConditions() = {22.3, 47.0, 749.9};
    CheckRange range;
    range.t_start = QDateTime::fromMSecsSinceEpoch(kT0 + 1000);
    range.t_end = QDateTime::fromMSecsSinceEpoch(kT0 + 3000);
    range.t_mid = QDateTime::fromMSecsSinceEpoch(kT0 + 2000);
    range.avg_temp = 22.125;
    range.avg_bar = 7.5;
    db.GetCheckRanges().push_back(range);
    const QDateTime x_min = QDateTime::fromMSecsSinceEpoch(kT0);
    const QDateTime x_max = QDateTime::fromMSecsSinceEpoch(kT0 + 4000);
    axis_x.setRange(x_min, x_max);
    db.SetDefaultAxisX(x_max, x_min);

    QVector<DataSeriesSensor> sensors;
    DataSeriesSensor sensor;
    sensor.name_sensor = "АЦМ-1";
    sensor.number_sensor = "№4217";
    sensor.vec_canal.push_back(MakeCanal("АЦМ-1", "Давление", 2, true));
    sensor.vec_canal.push_back(MakeCanal("АЦМ-1", "Температура", 3, false));
    sensors.push_back(sensor);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("doc.smv");
    dow.SaveSVDoc(path, sensors);
    QVERIFY2(QFile::exists(path), "документ не создан");

    // --- чистое состояние --------------------------------------------------
    const QVector<DataSeriesEtalon> expected_etalon = db.GetDataSerEtalon();
    const QVector<double> expected_cp_temp = db.GetCheckPointTemp();
    const QVector<double> expected_cp_bar = db.GetCheckPointBar();
    const QVector<QDateTime> expected_cp_times = db.GetCheckPoints();

    db.GetDataSerEtalon().clear();
    db.GetDataSerACM().clear();
    db.ClearAll();
    dow.ClearAll();
    axis_x.setRange(QDateTime::fromMSecsSinceEpoch(0), QDateTime::fromMSecsSinceEpoch(1));

    // --- загрузка ----------------------------------------------------------
    QVector<DataSeriesSensor> loaded;
    dow.LoadSVDoc(path, loaded);

    // Эталон.
    QCOMPARE(db.GetDataSerEtalon().size(), expected_etalon.size());
    for(int i = 0; i < expected_etalon.size(); ++i){
        const DataSeriesEtalon& want = expected_etalon[i];
        const DataSeriesEtalon& got = db.GetDataSerEtalon()[i];
        QCOMPARE(got.name_series, want.name_series);
        QCOMPARE(got.points_triangle_view, want.points_triangle_view);
        QCOMPARE(got.points_rectangle_view, want.points_rectangle_view);
        QCOMPARE(got.condition, want.condition);
        QVERIFY(got.series != nullptr);
        QVERIFY(got.axis_y_ != nullptr);
        QCOMPARE(got.axis_y_->min(), want.axis_y_->min());
        QCOMPARE(got.axis_y_->max(), want.axis_y_->max());
        QCOMPARE(got.series->count(), want.points_rectangle_view.size());
    }

    // Контрольные точки, диапазоны, условия, ось времени.
    QCOMPARE(db.GetCheckPointTemp(), expected_cp_temp);
    QCOMPARE(db.GetCheckPointBar(), expected_cp_bar);
    QCOMPARE(db.GetCheckPoints(), expected_cp_times);
    QCOMPARE(db.GetCheckPoints().size(), 3);
    // Производные от КТ данные восстановлены: зеркало времён и маркеры.
    QCOMPARE(db.GetCheckPoints64().size(), expected_cp_times.size());
    for(int i = 0; i < expected_cp_times.size(); ++i){
        QCOMPARE(db.GetCheckPoints64()[i], expected_cp_times[i].toMSecsSinceEpoch());
    }
    QCOMPARE(db.GetDataSerEtalon()[0].point_series->count(), expected_cp_times.size());
    QCOMPARE(db.GetDataSerEtalon()[1].point_series->count(), expected_cp_times.size());
    QCOMPARE(db.GetDataSerEtalon()[0].point_series->at(2).y(), 23.5);
    QCOMPARE(db.GetDataSerEtalon()[1].point_series->at(2).y(), 8.5);
    QCOMPARE(db.GetCheckRanges().size(), 1);
    QCOMPARE(db.GetCheckRanges()[0].t_start, range.t_start);
    QCOMPARE(db.GetCheckRanges()[0].t_end, range.t_end);
    QCOMPARE(db.GetCheckRanges()[0].t_mid, range.t_mid);
    QCOMPARE(db.GetCheckRanges()[0].avg_temp, range.avg_temp);
    QCOMPARE(db.GetCheckRanges()[0].avg_bar, range.avg_bar);
    QCOMPARE(db.GetConditions(), QVector<double>({22.3, 47.0, 749.9}));
    QCOMPARE(axis_x.min(), x_min);
    QCOMPARE(axis_x.max(), x_max);
    QCOMPARE(db.GetDefaultAxisX().first, x_min);
    QCOMPARE(db.GetDefaultAxisX().second, x_max);

    // Приборы: все каналы, включая невыбранные.
    QCOMPARE(loaded.size(), 1);
    QCOMPARE(loaded[0].name_sensor, sensor.name_sensor);
    QCOMPARE(loaded[0].number_sensor, sensor.number_sensor);
    QCOMPARE(loaded[0].vec_canal.size(), 2);
    for(int i = 0; i < sensor.vec_canal.size(); ++i){
        CompareCanal(loaded[0].vec_canal[i], sensor.vec_canal[i]);
        QVERIFY2(loaded[0].vec_canal[i].series != nullptr, "серия канала не создана");
        QVERIFY2(loaded[0].vec_canal[i].axis_y_ != nullptr, "ось канала не создана");
        QVERIFY2(loaded[0].vec_canal[i].label_data != nullptr, "метка канала не создана");
        QVERIFY2(!loaded[0].vec_canal[i].flag_setting_canal, "канал помечен как уже в легенде");
    }

    // Второй круг: пересохранение загруженного документа даёт тот же файл.
    const QString path2 = dir.filePath("doc2.smv");
    dow.SaveSVDoc(path2, loaded);
    QFile f1(path), f2(path2);
    QVERIFY(f1.open(QIODevice::ReadOnly) && f2.open(QIODevice::ReadOnly));
    QCOMPARE(f2.readAll(), f1.readAll());
}

// Кнопка «Сохранить» пишет не all_data_sensor_, а слияние его с моделью:
// настройки и рассчитанные погрешности живут в модели, невыбранные каналы —
// только в полном списке.
void TstSvFormat::snapshotTakesRuntimeState(){
    QVector<DataSeriesSensor> all_sensors;
    DataSeriesSensor sensor;
    sensor.name_sensor = "АЦМ-1";
    sensor.number_sensor = "№4217";
    sensor.vec_canal.push_back(MakeCanal("АЦМ-1", "Давление", 2, true));
    sensor.vec_canal.push_back(MakeCanal("АЦМ-1", "Температура", 3, false));
    all_sensors.push_back(sensor);

    // Модель: только выбранный канал, но с рассчитанными таблицей погрешностей
    // check_points/delta_points, которых в полном списке нет.
    QVector<DataSeriesSensor> model;
    DataSeriesSensor in_model;
    in_model.name_sensor = "АЦМ-1";
    Canal live = MakeCanal("АЦМ-1", "Давление", 2, true);
    live.check_points = {11.0, 22.0};
    live.delta_points = {0.11, 0.22};
    live.color_series_RGB = "0,0,255";
    in_model.vec_canal.push_back(live);
    model.push_back(in_model);

    // Прибор, которого нет в полном списке, теряться не должен.
    DataSeriesSensor orphan;
    orphan.name_sensor = "АЦМ-2";
    orphan.vec_canal.push_back(MakeCanal("АЦМ-2", "Давление", 1, true));
    model.push_back(orphan);

    const QVector<DataSeriesSensor> snapshot = SnapshotSensors(all_sensors, model);

    QCOMPARE(snapshot.size(), 2);
    QCOMPARE(snapshot[0].vec_canal.size(), 2);
    // Выбранный канал взят из модели.
    QCOMPARE(snapshot[0].vec_canal[0].check_points, QVector<double>({11.0, 22.0}));
    QCOMPARE(snapshot[0].vec_canal[0].delta_points, QVector<double>({0.11, 0.22}));
    QCOMPARE(snapshot[0].vec_canal[0].color_series_RGB, QString("0,0,255"));
    // Невыбранный канал остался из полного списка.
    QCOMPARE(snapshot[0].vec_canal[1].name_canal, QString("Температура"));
    QVERIFY(!snapshot[0].vec_canal[1].select_box);
    QCOMPARE(snapshot[0].vec_canal[1].check_points, sensor.vec_canal[1].check_points);
    // Прибор только из модели добавлен целиком.
    QCOMPARE(snapshot[1].name_sensor, QString("АЦМ-2"));
    QCOMPARE(snapshot[1].vec_canal.size(), 1);
}

QTEST_MAIN(TstSvFormat)
#include "tst_svformat.moc"
