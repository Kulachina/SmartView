#include "mainwindow.h"
#include "licensemanager.h"
#include "licensedialog.h"
#include <QTimer>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("SmartView");
    a.setApplicationName("SmartView");
    a.setWindowIcon(QIcon(":/SmartView.ico"));

    // Проверка лицензии. Пока License::kEnforce == false — приложение запускается как обычно
    // (сервер ещё не настроен). После публикации сервера задайте kServerUrl и kEnforce = true.
    if(License::kEnforce){
        LicenseManager lm;
        if(lm.CheckAtStartup() != LicenseManager::Status::Valid){
            LicenseDialog dlg(&lm);
            if(dlg.exec() != QDialog::Accepted){
                return 0;   // без действующей лицензии не запускаемся
            }
        }
    }

    MainWindow w;
    w.setWindowTitle("SmartView");
    w.resize(800,600);
    w.show();
    return a.exec();
}
