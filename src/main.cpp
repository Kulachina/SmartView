#include "mainwindow.h"
#include <QTimer>
#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>

namespace {
bool InstallRussian(QApplication& app){
    static QTranslator translator;
    const QStringList dirs = {QApplication::applicationDirPath() + "/translations",
                              QLibraryInfo::path(QLibraryInfo::TranslationsPath)};
    const QStringList names = {"qtbase_ru", "qt_ru"};
    for(const QString& dir : dirs){
        for(const QString& name : names){
            if(translator.load(name, dir)){
                return app.installTranslator(&translator);
            }
        }
    }
    return false;
}
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("SmartView");
    a.setApplicationName("SmartView");
    a.setWindowIcon(QIcon(":/SmartView.ico"));
    InstallRussian(a);

    MainWindow w;
    w.setWindowTitle("SmartView");
    w.resize(800,600);
    w.show();
    return a.exec();
}
