#include "mainwindow.h"
#include "licensemanager.h"
#include "licensedialog.h"
#include <QTimer>
#include <QApplication>
#include <QMessageBox>

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
        const LicenseManager::Status st = lm.CheckAtStartup();
        if(st == LicenseManager::Status::Error){
            // Давно не было связи с сервером — без проверки не запускаем.
            QMessageBox::warning(nullptr, "Лицензия",
                "Не удалось проверить лицензию: нет связи с сервером.\n"
                "Подключитесь к интернету и запустите программу снова.");
            return 0;
        }
        if(st != LicenseManager::Status::Valid){
            if(st == LicenseManager::Status::Expired){
                QMessageBox::warning(nullptr, "Лицензия",
                    "Срок действия лицензии истёк.\nОбратитесь к поставщику ПО.");
            }
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
