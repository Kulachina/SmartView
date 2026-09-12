#include "protocolwriter.h"
#include "util.h"
#include "xlsxcell.h"
#include "xlsxcellrange.h"
#include "xlsxdocument.h"
#include "xlsxformat.h"
#include "xlsxworksheet.h"
#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QDate>
#include <QFile>
#include <QFontMetrics>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include <QVector>

using namespace QXlsx;



namespace {
// «Заводской номер»: столько же знаков, сколько принимает бланк.
constexpr int kSerialMaxLen = 10;

// Квадратная кнопка «+» справа от пополняемого списка: side — высота поля,
// рядом с которым она стоит, чтобы кнопки выстроились в одну колонку.
QToolButton* MakeAddButton(int side) {
    QToolButton* btn = new QToolButton();
    btn->setText(QStringLiteral("+"));
    btn->setToolTip(QStringLiteral("Добавить пункт в список"));
    btn->setFixedSize(side, side);
    return btn;
}
}

ProtocolWriter::ProtocolWriter(DataBase& data_base,QWidget* parent)
    : QWidget(parent), data_base_(data_base)
{
    setWindowTitle("Настройка протокола");

    select_pribor_ = new QComboBox();
    select_name_type_ = new QComboBox();
    select_name_type_->addItems(catalog_.Items(ProtocolCatalog::kDeviceNames));
    type_pribor_ = new QComboBox();
    type_pribor_->addItems(catalog_.Items(ProtocolCatalog::kDeviceTypes));
    list_client_ = new QComboBox();
    list_client_->addItems(catalog_.Items(ProtocolCatalog::kCustomers));
    for(QComboBox* box : {select_pribor_, select_name_type_, type_pribor_, list_client_}){
        box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    series_number_ = new QLineEdit();
    series_number_->setMaxLength(kSerialMaxLen);
    series_number_->setAlignment(Qt::AlignLeft);
    // Поле под десять цифр во всю ширину окна выглядит случайным — даём ему
    // ширину по содержимому с небольшим запасом.
    series_number_->setMaximumWidth(QFontMetrics(series_number_->font())
                                        .horizontalAdvance(QString(kSerialMaxLen + 4, u'0')));
    connect(select_pribor_, QOverload<int>::of(&QComboBox::activated),
            this, [this](int index) {
                const QString name = select_pribor_->itemText(index);
                if (map_pribors_.contains(name)) {
                    series_number_->setText(map_pribors_.value(name));
                    series_number_->setReadOnly(true);
                } else {
                    series_number_->clear();
                    series_number_->setReadOnly(false);
                    series_number_->setFocus();
                }
            });

    // Форма держит подписи и поля в двух колонках: при отдельных строчных
    // раскладках каждое поле начиналось со своего отступа, по длине подписи.
    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignLeft);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->addRow("Выбор прибора", select_pribor_);
    form->addRow("Наименование аппаратуры",
                 WithAddButton(select_name_type_, ProtocolCatalog::kDeviceNames,
                               "Наименование аппаратуры"));
    form->addRow("Тип аппаратуры",
                 WithAddButton(type_pribor_, ProtocolCatalog::kDeviceTypes,
                               "Тип аппаратуры"));
    form->addRow("Заводской номер", series_number_);
    form->addRow("Заказчик",
                 WithAddButton(list_client_, ProtocolCatalog::kCustomers, "Заказчик"));

    QGroupBox *instr_group = new QGroupBox("Средства калибровки");
    instr_layout_ = new QVBoxLayout(instr_group);
    for (const QString& name : catalog_.Items(ProtocolCatalog::kCalibMeans)) {
        QCheckBox* box = new QCheckBox(name);
        instr_boxes_.push_back(box);
        instr_layout_->addWidget(box);
    }
    if (!instr_boxes_.isEmpty())
        instr_boxes_.front()->setChecked(true);   // как раньше в списке — первый пункт
    // Кнопка «+» последней строкой группы — новые пункты встают перед ней.
    QToolButton *instr_add = MakeAddButton(select_name_type_->sizeHint().height());
    QHBoxLayout *instr_add_row = new QHBoxLayout();
    instr_add_row->setContentsMargins(0, 0, 0, 0);
    instr_add_row->addStretch(1);
    instr_add_row->addWidget(instr_add);
    instr_layout_->addLayout(instr_add_row);
    connect(instr_add, &QToolButton::clicked, this, &ProtocolWriter::AddInstrument);

    canal_temp_ = new QCheckBox("Температура");
    canal_bar_ = new QCheckBox("Давление");
    auto make_spin = [](int value){
        QSpinBox* spin = new QSpinBox();
        spin->setRange(1, 20);
        spin->setValue(value);
        spin->setMinimumWidth(60);
        return spin;
    };
    temp_points_spin_ = make_spin(1);
    bar_points_spin_ = make_spin(3);
    bar_temp_points_spin_ = make_spin(1);
    temp_points_label_ = new QLabel("Точек температуры");
    bar_points_label_ = new QLabel("Точек давления");
    bar_temp_points_label_ = new QLabel("Точек температуры");

    // Сетка, а не вложенные строки: счётчики обоих каналов встают в одну
    // колонку независимо от длины подписи слева.
    QGroupBox *canal_group = new QGroupBox("Выбор каналов для протокола");
    QGridLayout *canal_layout = new QGridLayout(canal_group);
    canal_layout->addWidget(canal_temp_, 0, 0);
    canal_layout->addWidget(temp_points_label_, 0, 1);
    canal_layout->addWidget(temp_points_spin_, 0, 2);
    canal_layout->addWidget(canal_bar_, 1, 0);
    canal_layout->addWidget(bar_points_label_, 1, 1);
    canal_layout->addWidget(bar_points_spin_, 1, 2);
    canal_layout->addWidget(bar_temp_points_label_, 1, 3);
    canal_layout->addWidget(bar_temp_points_spin_, 1, 4);
    canal_layout->setColumnMinimumWidth(0, 140);
    canal_layout->setColumnStretch(5, 1);
    // Счётчики видны только у отмеченных каналов.
    const QList<QWidget*> temp_extra = {temp_points_label_, temp_points_spin_};
    for(QWidget* widget : temp_extra){
        widget->setVisible(false);
        connect(canal_temp_, &QCheckBox::toggled, widget, &QWidget::setVisible);
    }
    const QList<QWidget*> bar_extra = {bar_points_label_, bar_points_spin_,
                                       bar_temp_points_label_, bar_temp_points_spin_};
    for(QWidget* widget : bar_extra){
        widget->setVisible(false);
        connect(canal_bar_, &QCheckBox::toggled, widget, &QWidget::setVisible);
    }

    QPushButton *btn = new QPushButton("Создать");
    connect(btn, &QPushButton::clicked, this, [this]() {
        if (Generate(CollectData(), this))
            close();
    });
    QPushButton *btn_2 = new QPushButton("Отмена");
    connect(btn_2,&QPushButton::clicked, this, &QWidget::close);
    QHBoxLayout *h_btn = new QHBoxLayout();
    h_btn->addStretch(1);
    h_btn->addWidget(btn);
    h_btn->addWidget(btn_2);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addLayout(form);
    vbox->addWidget(instr_group);
    vbox->addWidget(canal_group);
    vbox->addStretch(1);
    vbox->addLayout(h_btn);
    setMinimumWidth(560);
}

void ProtocolWriter::GetSpisokPribors(){
    pribors_.clear();
    map_pribors_.clear();
    if(!data_base_.GetDataSerACM().isEmpty()){
        for(DataSeriesSensor a : data_base_.GetDataSerACM()){
            pribors_.push_back(a.name_sensor);
            map_pribors_[a.name_sensor] = TrimNameandNumber(a.name_sensor);
        }
        select_pribor_->addItems(pribors_);
    }
}
// --- пополняемые списки ---------------------------------------------------

QWidget* ProtocolWriter::WithAddButton(QComboBox* box, const QString& key,
                                       const QString& title) {
    QToolButton* add = MakeAddButton(box->sizeHint().height());
    connect(add, &QToolButton::clicked, this, [this, box, key, title]() {
        bool ok = false;
        const QString value = QInputDialog::getText(this, title, "Новый пункт:",
                                                    QLineEdit::Normal, QString(), &ok)
                                  .trimmed();
        if (!ok || value.isEmpty())
            return;
        const int existing = box->findText(value);
        if (existing >= 0) {           // такой пункт уже есть — просто выбираем его
            box->setCurrentIndex(existing);
            return;
        }
        if (!StoreNewItem(key, value))
            return;
        box->addItem(value);
        box->setCurrentIndex(box->count() - 1);
    });
    QWidget* row = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    layout->addWidget(box, 1);
    layout->addWidget(add);
    return row;
}

void ProtocolWriter::AddInstrument() {
    bool ok = false;
    const QString value = QInputDialog::getText(this, "Средства калибровки", "Новый пункт:",
                                                QLineEdit::Normal, QString(), &ok)
                              .trimmed();
    if (!ok || value.isEmpty())
        return;
    for (QCheckBox* box : instr_boxes_) {
        if (box->text() == value) {    // такой пункт уже есть — просто отмечаем его
            box->setChecked(true);
            return;
        }
    }
    if (!StoreNewItem(ProtocolCatalog::kCalibMeans, value))
        return;
    QCheckBox* box = new QCheckBox(value);
    box->setChecked(true);
    instr_boxes_.push_back(box);
    // Последняя строка группы — кнопка «+», новый пункт встаёт перед ней.
    instr_layout_->insertWidget(instr_layout_->count() - 1, box);
}

bool ProtocolWriter::StoreNewItem(const QString& key, const QString& value) {
    switch (catalog_.Add(key, value)) {
    case ProtocolCatalog::AddResult::Added:
        return true;
    case ProtocolCatalog::AddResult::SaveFailed:
        // В текущем сеансе пункт доступен, но следующий запуск его не увидит.
        QMessageBox::warning(this, "Настройка протокола",
                             "Не удалось сохранить список в файл\n" +
                                 catalog_.FilePath() +
                                 "\nПункт будет доступен только до закрытия программы.");
        return true;
    case ProtocolCatalog::AddResult::Empty:
    case ProtocolCatalog::AddResult::Duplicate:
        break;
    }
    return false;
}

QString ProtocolWriter::TrimNameandNumber(QString name){
    static const QRegularExpression re(QStringLiteral("№\\s*(\\d+)"));
    QRegularExpressionMatch match = re.match(name);
    if (match.hasMatch())
        return match.captured(1);
    return "err";
}


// =====================================================================
// Вёрстка результатной части протокола
//
// Всё ниже строки 60 строится кодом: набор каналов, число блоков давления и
// число точек переменные, а QXlsx не умеет вставлять/удалять строки — значит
// статической разметки в шаблоне там быть не может. Стили берём с листа-донора
// «_proto», куда при перекройке шаблона переехали исходные строки бланка
// (номера строк сохранены — отсюда «говорящие» константы ниже).
// =====================================================================
namespace {

const QString kMainSheet  = QStringLiteral("Сертификат");   // сам протокол
const QString kProtoSheet = QStringLiteral("_proto");       // скрытый донор стилей

// Строки-образцы на листе «_proto».
enum ProtoRow {
    kRowResults   = 61,   // «Результаты калибровки:»
    kRowCheck     = 63,   // «Внешний осмотр:» (подпись в A, значение в G)
    kRowSection   = 67,   // «N. Канал измерения ...»
    kRowSubTitle  = 69,   // «N.1 Результаты определения ...»
    kRowPHead     = 71,   // шапка таблицы давления (блок из 9 колонок)
    kRowPCaption  = 72,   // «Измерения при N °С»
    kRowPData     = 74,   // строка данных давления
    kRowPDataLast = 77,   // последняя строка блока — толстая нижняя граница
    kRowLimit     = 79,   // «Предел допускаемой ...»
    kRowTHead     = 85,   // шапка таблицы температуры (блок из 10 колонок)
    kRowTFirst    = 86,   // первая строка данных температуры
    kRowTData     = 87,   // последующие строки данных температуры
    kRowVerdict   = 92,   // заключение (объединение на 3 строки)
    kRowSignLine  = 96,   // линии подписи: ФИО и дата
    kRowSignCapt  = 97,   // подписи под линиями: Подпись / ФИО / Дата
};

const int kFirstRow     = 61;   // первая строка ниже фиксированной шапки
const int kLastCol      = 29;   // AC
const int kPBlockWidth  = 9;    // блок давления: эталон(3) + прибор(3) + погрешность(3)
const int kPBlocksInRow = 3;    // блоков давления по ширине листа
const int kTBlockWidth  = 10;   // таблица температуры: 3 + 3 + 4
const int kMeansRows    = 4;    // строк под средства калибровки в шапке бланка
const int kMeansFirstRow = 38;  // первая из них — G38, дальше через строку
const int kMeansCol     = 7;    // G

// Форматы одной строки-образца: fmt[k] — формат колонки k+1.
struct DonorRow {
    QVector<QXlsx::Format> fmt;
    double height = 0.0;
};
using Donors = QMap<int, DonorRow>;

// Читает все нужные строки-образцы разом, чтобы дальше не переключать лист.
Donors LoadDonors(QXlsx::Document& doc) {
    static const QList<int> rows = {kRowResults, kRowCheck, kRowSection, kRowSubTitle,
                                    kRowPHead, kRowPCaption, kRowPData, kRowPDataLast,
                                    kRowLimit, kRowTHead, kRowTFirst, kRowTData,
                                    kRowVerdict, kRowSignLine, kRowSignCapt};
    Donors res;
    doc.selectSheet(kProtoSheet);
    for (int r : rows) {
        DonorRow d;
        d.height = doc.rowHeight(r);
        d.fmt.resize(kLastCol);
        for (int c = 1; c <= kLastCol; ++c) {
            const auto cell = doc.cellAt(r, c);
            if (cell)
                d.fmt[c - 1] = cell->format();
        }
        res.insert(r, d);
    }
    doc.selectSheet(kMainSheet);
    return res;
}

// Пишет value в (row, col) и растягивает стили образца на width колонок.
// Формат берётся ПОКОЛОНОЧНО — так левая/средняя/правая границы ячеек совпадают
// с бланком. Объединяем БЕЗ формата: QXlsx::mergeCells с валидным форматом
// затирает им все ячейки диапазона и границы таблицы разъезжаются.
void PutRun(QXlsx::Document& doc, const DonorRow& donor, int row, int col, int width,
            const QVariant& value = QVariant(), int donor_col = 1) {
    for (int k = 0; k < width; ++k)
        doc.write(row, col + k, k == 0 ? value : QVariant(), donor.fmt.value(donor_col - 1 + k));
    if (width > 1)
        doc.mergeCells(QXlsx::CellRange(row, col, row, col + width - 1));
    if (donor.height > 0.0)
        doc.setRowHeight(row, donor.height);
}

// Одна ячейка со стилем образца, без объединения.
void PutCell(QXlsx::Document& doc, const DonorRow& donor, int row, int col,
             const QVariant& value, int donor_col) {
    doc.write(row, col, value, donor.fmt.value(donor_col - 1));
    if (donor.height > 0.0)
        doc.setRowHeight(row, donor.height);
}

// Значение в ячейку уже свёрстанной шапки. Пустое не пишем, чтобы не затереть
// текст, заданный прямо в шаблоне (место проведения, средства калибровки).
// Формат не передаём: QXlsx в этом случае сохраняет стиль ячейки шаблона.
void PutMeta(QXlsx::Document& doc, const QString& ref, const QString& value) {
    if (!value.isEmpty())
        doc.write(ref, value);
}

QString ErrName(Protocol::ErrType t) {          // для заголовка колонки
    switch (t) {
        case Protocol::ErrType::Absolute: return QStringLiteral("Абсолютная");
        case Protocol::ErrType::Relative: return QStringLiteral("Относительная");
        case Protocol::ErrType::Reduced:  return QStringLiteral("Приведенная");
    }
    return QString();
}

QString ErrNameGen(Protocol::ErrType t) {       // для строки «Предел допускаемой ...»
    switch (t) {
        case Protocol::ErrType::Absolute: return QStringLiteral("абсолютной");
        case Protocol::ErrType::Relative: return QStringLiteral("относительной");
        case Protocol::ErrType::Reduced:  return QStringLiteral("приведенной");
    }
    return QString();
}

// Число для текста (не для ячейки): без хвостовых нулей, с запятой как в бланке.
QString Num(double v) {
    return QString::number(v, 'g', 10).replace(QLatin1Char('.'), QLatin1Char(','));
}

QString ErrHeader(const Protocol::ChannelSpec& s) {
    return QString("%1 погрешность, %2").arg(ErrName(s.error_type), s.error_unit);
}

QString LimitText(const QString& what, const Protocol::ChannelSpec& s) {
    return QString("Предел допускаемой основной %1 погрешности измерения %2 ±%3 %4")
        .arg(ErrNameGen(s.error_type), what, Num(s.error_limit), s.error_unit);
}

// Значение точки в ячейку: незаполненная точка (measured == false) даёт пустую
// ячейку со стилем таблицы — протокол печатается как бланк под ручной ввод.
QVariant PointCell(bool measured, double value) {
    return measured ? QVariant(value) : QVariant();
}

// --- снятие показаний прибора в контрольных точках ---------------------------
// Идём по серии канала и берём y там, где время совпало с очередной КТ.
// Тот же механизм, что в MasterPointsWindow::AnalisingSeries: КТ — это
// пересечение отмеченных моментов с графиком прибора, время сравнивается
// округлённым до секунды. Эталон брать неоткуда не нужно: его значения в тех
// же точках DataBase хранит параллельно самим КТ (AddCheckPoint).
QVector<double> ReadCanalAtCheckPoints(QLineSeries* series,
                                       const QVector<QDateTime>& check_points) {
    QVector<double> res;
    if (!series)
        return res;
    int next = 0;
    for (const QPointF& point : series->points()) {
        if (next >= check_points.size())
            break;
        const qint64 t  = RoundToSec(static_cast<qint64>(point.x()));
        const qint64 cp = RoundToSec(check_points[next].toMSecsSinceEpoch());
        if (t == cp) {
            res.push_back(point.y());
            ++next;
        }
    }
    return res;
}

// --- характеристики канала из настроек прибора -------------------------------
// Тип погрешности задаётся в окне «Приборы и каналы» (SensorCanalEditor):
// 1 — абсолютная, 2 — относительная, 3 — приведённая; 0 — канал не настроен.
Protocol::ErrType ErrTypeFromCanal(int type_error) {
    switch (type_error) {
        case 1:  return Protocol::ErrType::Absolute;
        case 2:  return Protocol::ErrType::Relative;
        default: return Protocol::ErrType::Reduced;   // 3
    }
}

// В бланке единицы набраны со степенью, в списке окна каналов — плоско.
QString PrettyUnit(QString unit) {
    return unit.replace(QStringLiteral("см2"), QStringLiteral("см²"));
}

// Переносит настройки канала в характеристики протокола. Единица погрешности:
// у абсолютной — единица канала, у относительной и приведённой — проценты.
// Диапазон для приведённой считается так же, как в ErrorTable::AnalisingSeries:
// duration_error_max - duration_error_min.
void SpecFromCanal(const Canal& c, Protocol::ChannelSpec& s) {
    s.unit        = PrettyUnit(c.name_unit);
    s.error_type  = ErrTypeFromCanal(c.type_error);
    s.error_limit = c.accept_max;
    s.error_unit  = s.error_type == Protocol::ErrType::Absolute ? s.unit
                                                                : QStringLiteral("%");
    const double span = c.duration_error_max - c.duration_error_min;
    s.span = span != 0.0 ? span : 1.0;
}

// --- верхний колонтитул -----------------------------------------------------
// Нижний колонтитул (о запрете воспроизведения) статический и живёт прямо в
// шаблоне. Верхний приходится собирать: в шаблоне остались номер и дата из
// образца, а они у каждого протокола свои. Нотация Excel: &R — правая секция,
// &P — номер страницы, &N — всего страниц.
QString EscapeHeader(QString text) {
    return text.replace(QLatin1Char('&'), QStringLiteral("&&"));   // & — префикс кода
}

QString HeaderText(const Protocol::Data& d) {
    QString title;
    if (!d.number.isEmpty() && !d.date.isEmpty())
        title = QString("Протокол № %1 от %2").arg(EscapeHeader(d.number), EscapeHeader(d.date));
    else if (!d.number.isEmpty())
        title = QString("Протокол № %1").arg(EscapeHeader(d.number));
    else if (!d.date.isEmpty())
        title = QString("Протокол от %1").arg(EscapeHeader(d.date));
    if (!title.isEmpty())
        title += QStringLiteral("        ");
    return QStringLiteral("&R") + title + QStringLiteral("стр &P из &N");
}

// --- метаданные шапки (строки 1..60 шаблона) -------------------------------
void WriteMeta(QXlsx::Document& doc, const Protocol::Data& d) {
    PutMeta(doc, QStringLiteral("L9"),  d.number);
    PutMeta(doc, QStringLiteral("P9"),  d.date);
    PutMeta(doc, QStringLiteral("K11"), d.device_name);
    PutMeta(doc, QStringLiteral("K13"), d.device_type);
    PutMeta(doc, QStringLiteral("K15"), d.serial);
    PutMeta(doc, QStringLiteral("K17"), d.customer);
    PutMeta(doc, QStringLiteral("K19"), d.basis);
    PutMeta(doc, QStringLiteral("K30"), d.period);
    PutMeta(doc, QStringLiteral("T26"), d.ambient_temp);
    PutMeta(doc, QStringLiteral("T27"), d.humidity);
    PutMeta(doc, QStringLiteral("T28"), d.atm_pressure);

    const QStringList place = d.location.split(QLatin1Char('\n'));
    PutMeta(doc, QStringLiteral("K21"), place.value(0));
    PutMeta(doc, QStringLiteral("K22"), place.value(1));

    // Средства калибровки — G38/G40/G42/G44, через строку. Отмечено может быть
    // сколько угодно пунктов, но в бланке под них kMeansRows строк: сначала
    // чистим все, иначе от прошлого текста шаблона останется смесь.
    if (!d.calib_means.isEmpty()) {
        for (int i = 0; i < kMeansRows; ++i)
            doc.write(kMeansFirstRow + i * 2, kMeansCol, QVariant());
        const int count = qMin(int(d.calib_means.size()), kMeansRows);
        for (int i = 0; i < count; ++i)
            doc.write(kMeansFirstRow + i * 2, kMeansCol, d.calib_means[i]);
    }
}

// --- «Результаты калибровки» + внешний осмотр и опробование ----------------
int WriteResultsHead(QXlsx::Document& doc, const Donors& dn, const Protocol::Data& d, int row) {
    PutRun(doc, dn[kRowResults], row, 1, kLastCol, QStringLiteral("Результаты калибровки:"));
    row += 2;
    PutCell(doc, dn[kRowCheck], row, 1, QStringLiteral("Внешний осмотр:"), 1);
    PutCell(doc, dn[kRowCheck], row, 7, d.external_inspection, 7);
    row += 2;
    PutCell(doc, dn[kRowCheck], row, 1, QStringLiteral("Опробование:"), 1);
    PutCell(doc, dn[kRowCheck], row, 7, d.trial, 7);
    return row + 2;
}

// --- канал давления: блоки по kPBlocksInRow в ряд, ниже — следующий ряд -----
int WritePressureChannel(QXlsx::Document& doc, const Donors& dn, const Protocol::Data& d,
                         int row, int section) {
    const Protocol::ChannelSpec& s = d.pressure_spec;
    PutRun(doc, dn[kRowSection], row, 1, kLastCol,
           QString("%1. Канал измерения давления").arg(section));
    row += 2;
    PutCell(doc, dn[kRowSubTitle], row, 1,
            QString("%1.1 Результаты определения метрологических характеристик:").arg(section), 1);
    row += 2;

    for (int first = 0; first < d.pressure_blocks.size(); first += kPBlocksInRow) {
        const int in_row = qMin(kPBlocksInRow, int(d.pressure_blocks.size()) - first);
        int points = 0;
        for (int b = 0; b < in_row; ++b)
            points = qMax(points, int(d.pressure_blocks[first + b].points.size()));

        for (int b = 0; b < in_row; ++b) {
            const int c0 = 1 + b * kPBlockWidth;
            const Protocol::PressureBlock& blk = d.pressure_blocks[first + b];
            PutRun(doc, dn[kRowPHead], row, c0,     3, "Показания эталона, " + s.unit, 1);
            PutRun(doc, dn[kRowPHead], row, c0 + 3, 3, "Показания прибора, " + s.unit, 4);
            PutRun(doc, dn[kRowPHead], row, c0 + 6, 3, ErrHeader(s), 7);
            // При пустом бланке температура блока ещё неизвестна — подпись
            // остаётся пустой, её впишут от руки.
            const QVariant caption = blk.temperature != 0.0
                ? QVariant(QString("Измерения при %1 °С").arg(Num(blk.temperature)))
                : QVariant();
            PutRun(doc, dn[kRowPCaption], row + 1, c0, kPBlockWidth, caption);
        }
        row += 2;

        for (int p = 0; p < points; ++p) {
            const DonorRow& donor = dn[p + 1 == points ? kRowPDataLast : kRowPData];
            for (int b = 0; b < in_row; ++b) {
                const int c0 = 1 + b * kPBlockWidth;
                const QVector<Protocol::Point>& pts = d.pressure_blocks[first + b].points;
                const bool has = p < pts.size() && pts[p].measured;
                PutRun(doc, donor, row, c0,     3, PointCell(has, has ? pts[p].reference : 0.0), 1);
                PutRun(doc, donor, row, c0 + 3, 3, PointCell(has, has ? pts[p].device : 0.0), 4);
                PutRun(doc, donor, row, c0 + 6, 3,
                       PointCell(has, has ? Protocol::CalcError(s, pts[p]) : 0.0), 7);
            }
            ++row;
        }
        ++row;   // пустая строка между рядами блоков
    }

    PutCell(doc, dn[kRowLimit], row, 1, LimitText(QStringLiteral("давления"), s), 1);
    return row + 2;
}

// --- канал температуры: одна таблица ---------------------------------------
int WriteTemperatureChannel(QXlsx::Document& doc, const Donors& dn, const Protocol::Data& d,
                            int row, int section) {
    const Protocol::ChannelSpec& s = d.temperature_spec;
    PutRun(doc, dn[kRowSection], row, 1, kLastCol,
           QString("%1. Канал измерения температуры").arg(section));
    row += 2;
    PutCell(doc, dn[kRowSubTitle], row, 1,
            QString("%1.1 Результаты определения метрологических характеристик:").arg(section), 1);
    row += 2;

    PutRun(doc, dn[kRowTHead], row, 1, 3, "Показания эталона, " + s.unit, 1);
    PutRun(doc, dn[kRowTHead], row, 4, 3, "Показания прибора, " + s.unit, 4);
    PutRun(doc, dn[kRowTHead], row, 7, kTBlockWidth - 6, ErrHeader(s), 7);
    ++row;

    for (int p = 0; p < d.temperature_points.size(); ++p) {
        const DonorRow& donor = dn[p == 0 ? kRowTFirst : kRowTData];
        const Protocol::Point& pt = d.temperature_points[p];
        PutRun(doc, donor, row, 1, 3, PointCell(pt.measured, pt.reference), 1);
        PutRun(doc, donor, row, 4, 3, PointCell(pt.measured, pt.device), 4);
        PutRun(doc, donor, row, 7, kTBlockWidth - 6,
               PointCell(pt.measured, Protocol::CalcError(s, pt)), 7);
        ++row;
    }
    ++row;
    PutCell(doc, dn[kRowLimit], row, 1, LimitText(QStringLiteral("температуры"), s), 1);
    return row + 2;
}

// --- заключение -------------------------------------------------------------
int WriteVerdict(QXlsx::Document& doc, const Donors& dn, const Protocol::Data& d, int row) {
    // Пока измерений нет — место под заключение остаётся пустым.
    QVariant text;
    if (Protocol::HasMeasurements(d)) {
        text = QString("По результатам калибровки аппаратура %1 зав.№ %2 соответствует заявленным техническим характеристикам. "
                       "По результатам проведенных испытаний погрешности показаний по каналам измерения давления и "
                       "температуры не превышают основную допускаемую погрешность.")
                   .arg(d.device_type, d.serial);
    }
    const DonorRow& donor = dn[kRowVerdict];
    for (int r = row; r < row + 3; ++r)
        for (int c = 1; c <= kLastCol; ++c)
            doc.write(r, c, (r == row && c == 1) ? text : QVariant(), donor.fmt.value(c - 1));
    doc.mergeCells(QXlsx::CellRange(row, 1, row + 2, kLastCol));
    return row + 4;
}

// --- подпись ----------------------------------------------------------------
int WriteSignature(QXlsx::Document& doc, const Donors& dn, const Protocol::Data& d, int row) {
    const DonorRow& line = dn[kRowSignLine];
    PutCell(doc, line, row, 1, QStringLiteral("Лицо, ответственное за калибровку"), 1);
    PutRun(doc, line, row, 11, 5, QVariant(), 11);        // K:O — место под подпись
    PutRun(doc, line, row, 18, 7, d.responsible, 18);     // R:X — ФИО
    PutRun(doc, line, row, 26, 4, d.date, 26);            // Z:AC — дата
    ++row;
    const DonorRow& capt = dn[kRowSignCapt];
    PutCell(doc, capt, row, 13, QStringLiteral("Подпись"), 13);   // M
    PutCell(doc, capt, row, 21, QStringLiteral("ФИО"), 21);       // U
    PutRun (doc, capt, row, 26, 4, QStringLiteral("Дата"), 26);   // Z:AC
    return row;
}

} // namespace

Protocol::Data ProtocolWriter::CollectData() const {
    Protocol::Data d;
    d.date        = QDate::currentDate().toString(QStringLiteral("dd.MM.yyyy"));
    d.device_name = select_name_type_->currentText();
    d.device_type = type_pribor_->currentText();
    d.serial      = series_number_->text();
    d.customer    = list_client_->currentText();
    for (QCheckBox* box : instr_boxes_) {
        if (box->isChecked())
            d.calib_means.push_back(box->text());
    }

    // Условия проведения калибровки приходят хвостом файла эталона (.sml2).
    // Старые файлы его не содержат — тогда ячейки шаблона остаются как есть.
    const QVector<double>& cond = data_base_.GetConditions();
    if (cond.size() == 3) {
        d.ambient_temp = Num(cond[0]);   // температура окружающей среды, °C
        d.humidity     = Num(cond[1]);   // относительная влажность воздуха, %
        d.atm_pressure = Num(cond[2]);   // атмосферное давление, мм.рт.ст
    }

    // Значения по умолчанию — на случай, если каналы прибора ещё не настроены
    // (type_error == 0): протокол всё равно должен получиться осмысленным.
    d.pressure_spec.unit        = QStringLiteral("кгс/см²");
    d.pressure_spec.error_type  = Protocol::ErrType::Reduced;
    d.pressure_spec.error_limit = 0.15;
    d.pressure_spec.error_unit  = QStringLiteral("%");
    d.pressure_spec.span        = 1000.0;

    d.temperature_spec.unit        = QStringLiteral("°С");
    d.temperature_spec.error_type  = Protocol::ErrType::Absolute;
    d.temperature_spec.error_limit = 1.5;
    d.temperature_spec.error_unit  = QStringLiteral("°С");
    d.temperature_spec.span        = 120.0;

    // Прибор, выбранный в форме: из него берём и настройки каналов, и показания.
    const DataSeriesSensor* sensor = nullptr;
    for (const DataSeriesSensor& s : data_base_.GetDataSerACM()) {
        if (s.name_sensor == select_pribor_->currentText()) {
            sensor = &s;
            break;
        }
    }

    // Эталон — значения в контрольных точках, DataBase держит их параллельно
    // самим КТ. Показания прибора снимаем с серий его каналов в те же моменты.
    // Канал определяется по имени, как в ErrorTable и «Мастере точек».
    const QVector<QDateTime>& cp      = data_base_.GetCheckPoints();
    const QVector<double>&    cp_bar  = data_base_.GetCheckPointBar();
    const QVector<double>&    cp_temp = data_base_.GetCheckPointTemp();
    QVector<double> dev_bar, dev_temp;
    if (sensor) {
        for (const Canal& c : sensor->vec_canal) {
            const bool is_bar =
                c.name_canal.contains(QStringLiteral("Давление"), Qt::CaseInsensitive);
            const bool is_temp =
                c.name_canal.contains(QStringLiteral("Температура"), Qt::CaseInsensitive);
            if (!is_bar && !is_temp)
                continue;
            // От spec зависят заголовок колонки погрешности, строка «Предел
            // допускаемой ...», сам расчёт погрешности и заключение. Канал без
            // настроек (type_error == 0) оставляет значение по умолчанию.
            if (c.type_error >= 1 && c.type_error <= 3)
                SpecFromCanal(c, is_bar ? d.pressure_spec : d.temperature_spec);
            (is_bar ? dev_bar : dev_temp) = ReadCanalAtCheckPoints(c.series, cp);
        }
    }

    // Галочки решают, какие разделы появятся в протоколе, счётчики — размер
    // таблиц. КТ уложены по температурным ступеням: сначала все точки давления
    // при первой температуре, потом при второй и т.д. — тот же порядок, что
    // и в «Мастере точек» (там эталонные температуры берутся с шагом,
    // равным числу точек давления). 5 температур x 5 давлений = 25 КТ.
    const int points_per_block = bar_points_spin_->value();
    if (canal_bar_->isChecked()) {
        for (int b = 0; b < bar_temp_points_spin_->value(); ++b) {
            const int base = b * points_per_block;
            Protocol::PressureBlock blk;
            blk.temperature = cp_temp.value(base, 0.0);   // подпись блока
            for (int i = 0; i < points_per_block; ++i) {
                const int k = base + i;
                Protocol::Point p;
                // Точек может не хватить: КТ ещё не расставлены или их меньше,
                // чем клеток. Тогда ячейка остаётся пустой под ручной ввод.
                p.measured = k < cp_bar.size() && k < dev_bar.size();
                if (p.measured) {
                    p.reference = cp_bar[k];
                    p.device    = dev_bar[k];
                }
                blk.points.push_back(p);
            }
            d.pressure_blocks.push_back(blk);
        }
    }
    if (canal_temp_->isChecked()) {
        // По одной точке на температурную ступень: если снимался и канал
        // давления, ступень — это каждая points_per_block-я КТ.
        const int stride = canal_bar_->isChecked() ? qMax(1, points_per_block) : 1;
        for (int i = 0; i < temp_points_spin_->value(); ++i) {
            const int k = i * stride;
            Protocol::Point p;
            p.measured = k < cp_temp.size() && k < dev_temp.size();
            if (p.measured) {
                p.reference = cp_temp[k];
                p.device    = dev_temp[k];
            }
            d.temperature_points.push_back(p);
        }
    }
    return d;
}

bool ProtocolWriter::Generate(const Protocol::Data& data, QWidget* parent,
                              const QString& save_path) {
    if (data.pressure_blocks.isEmpty() && data.temperature_points.isEmpty()) {
        QMessageBox::warning(parent, "Протокол",
                             "Не выбран ни один канал — протокол был бы пустым.");
        return false;
    }

    // ----------------------------------------------------------------
    // 1. Открытие шаблона из ресурсов (:/protocol_template.xlsx).
    //    Грузим через QBuffer (байты ресурса), чтобы не зависеть от того,
    //    умеет ли zip-ридер открывать путь вида ":/...".
    // ----------------------------------------------------------------
    QFile res(QStringLiteral(":/protocol_template.xlsx"));
    if (!res.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(parent, "Ошибка", "Шаблон протокола не найден в ресурсах.");
        return false;
    }
    QByteArray bytes = res.readAll();
    res.close();

    QBuffer buffer(&bytes);
    buffer.open(QIODevice::ReadOnly);
    QXlsx::Document doc(&buffer);
    if (!doc.load()) {
        QMessageBox::warning(parent, "Ошибка", "Не удалось открыть шаблон протокола.");
        return false;
    }
    if (!doc.selectSheet(kMainSheet)) {
        QMessageBox::warning(parent, "Ошибка", "В шаблоне нет листа «Сертификат».");
        return false;
    }

    // ----------------------------------------------------------------
    // 2. Вёрстка: шапка по фиксированным адресам, всё ниже — курсором.
    //    Номер раздела считается на месте, поэтому при снятой галочке
    //    «Давление» температура становится разделом 1, а не 2.
    // ----------------------------------------------------------------
    const Donors donors = LoadDonors(doc);
    WriteMeta(doc, data);

    int row = WriteResultsHead(doc, donors, data, kFirstRow);
    int section = 1;
    if (!data.pressure_blocks.isEmpty())
        row = WritePressureChannel(doc, donors, data, row, section++);
    if (!data.temperature_points.isEmpty())
        row = WriteTemperatureChannel(doc, donors, data, row, section++);
    row = WriteVerdict(doc, donors, data, row);
    row = WriteSignature(doc, donors, data, row);

    // Область печати — по фактической последней строке: в шаблоне она была
    // фиксированной ($A$1:$AC$98) и обрезала бы длинный протокол.
    doc.defineName(QStringLiteral("_xlnm.Print_Area"),
                   QString("'%1'!$A$1:$AC$%2").arg(kMainSheet).arg(row),
                   QString(), kMainSheet);

    // Донор стилей в готовом протоколе не нужен: форматы уже скопированы в
    // ячейки листа «Сертификат» и живут в общих стилях книги.
    doc.deleteSheet(kProtoSheet);
    doc.selectSheet(kMainSheet);

    // Верхний колонтитул — с фактическим номером и датой протокола.
    if (QXlsx::Worksheet* sheet = doc.currentWorksheet())
        sheet->setOddHeader(HeaderText(data));

    // ----------------------------------------------------------------
    // 3. Куда сохранять: спрашиваем путь у пользователя.
    //    Имя по умолчанию — из номера протокола, рядом с программой.
    // ----------------------------------------------------------------
    QString path = save_path;
    if (path.isEmpty()) {
        const QString suggested = QString("%1/Протокол %2.xlsx").arg(
            QApplication::applicationDirPath(),
            data.number.isEmpty() ? QStringLiteral("калибровки") : data.number);
        path = QFileDialog::getSaveFileName(
            parent, "Сохранить протокол", suggested, "Книга Excel (*.xlsx)");
    }
    if (path.isEmpty())
        return false;   // пользователь отменил сохранение
    if (!path.endsWith(".xlsx", Qt::CaseInsensitive))
        path += ".xlsx";

    if (!doc.saveAs(path)) {
        QMessageBox::warning(parent, "Ошибка",
                             "Не удалось сохранить протокол по пути:\n" + path);
        return false;
    }
    return true;
}
