#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/SmartView.ico"));
    MainWindow w;
    w.setWindowTitle("SmartView");
    w.resize(800,600);
    w.show();




    return a.exec();
}
