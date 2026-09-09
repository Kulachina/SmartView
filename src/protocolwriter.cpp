#include "protocolwriter.h"
#include "xlsxdocument.h"
#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>

using namespace QXlsx;

bool ProtocolWriter::Generate(const Protocol::Data& data, QWidget* parent) {
    // ----------------------------------------------------------------
    // 1. Открытие шаблона из ресурсов (:/protocol_template.xlsx).
    //    Грузим через QBuffer (байты ресурса), чтобы не зависеть от того,
    //    умеет ли zip-ридер открывать путь вида ":/...".
    // ----------------------------------------------------------------
    QFile res(":/protocol_template.xlsx");
    if (!res.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(parent, "Ошибка", "Шаблон протокола не найден в ресурсах.");
        return false;
    }
    QByteArray bytes = res.readAll();
    res.close();

    QBuffer buffer(&bytes);
    buffer.open(QIODevice::ReadOnly);
    Document doc(&buffer);
    if (!doc.load()) {
        QMessageBox::warning(parent, "Ошибка", "Не удалось открыть шаблон протокола.");
        return false;
    }
    if (!doc.selectSheet("Сертификат")) {
        QMessageBox::warning(parent, "Ошибка", "В шаблоне нет листа «Сертификат».");
        return false;
    }

    // ================================================================
    // 2. ВПИСЫВАНИЕ ЯЧЕЕК ИЗ data — реализуется отдельно.
    //    Лист «Сертификат» уже выбран, стили/лого/объединения сохранятся.
    //
    //    Запись значения (стиль ячейки сохраняется):
    //        doc.write("L9",  data.number);        // № протокола
    //        doc.write("P9",  data.date);          // дата
    //        doc.write("K13", data.device_type);   // тип аппаратуры
    //        doc.write("K15", data.serial);        // заводской номер
    //        doc.write("K17", data.customer);      // заказчик
    //
    //    Таблицы давления/температуры — динамически (переменное число
    //    блоков/точек): погрешность точки = Protocol::CalcError(spec, point);
    //    заключение (A92) по Protocol::IsWithinLimits(data).
    //    Для добавляемых строк копировать формат шаблонной строки и заново
    //    объединять ячейки: doc.mergeCells("A78:C78", fmt).
    // ================================================================
    (void)data;   // убрать, когда появится вписывание

    // ----------------------------------------------------------------
    // 3. Куда сохранять: спрашиваем путь у пользователя.
    //    Имя по умолчанию — из номера протокола, рядом с программой.
    // ----------------------------------------------------------------
    const QString suggested = QString("%1/Протокол %2.xlsx").arg(
        QApplication::applicationDirPath(),
        data.number.isEmpty() ? QStringLiteral("калибровки") : data.number);

    QString path = QFileDialog::getSaveFileName(
        parent, "Сохранить протокол", suggested, "Книга Excel (*.xlsx)");
    if (path.isEmpty()) {
        return false;   // пользователь отменил сохранение
    }
    if (!path.endsWith(".xlsx", Qt::CaseInsensitive)) {
        path += ".xlsx";
    }

    if (!doc.saveAs(path)) {
        QMessageBox::warning(parent, "Ошибка",
                             "Не удалось сохранить протокол по пути:\n" + path);
        return false;
    }
    return true;
}
