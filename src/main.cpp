#include "mainwindow.h"
#include "update.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/SmartView.ico"));
    MainWindow w;
    w.setWindowTitle("SmartView");
    w.resize(800,600);
    w.show();
    Updater updater;
    QObject::connect(&updater, &Updater::updateAvailable, [&updater](const QString &) {
        // Показываем системное уведомление (трей)
        // Или сохраняем флаг, чтобы показать при следующем запуске
        QSettings settings;
        settings.setValue("UpdateAvailable", true);
    });



    return a.exec();
}
