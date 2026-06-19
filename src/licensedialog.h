#ifndef LICENSEDIALOG_H
#define LICENSEDIALOG_H
#pragma once
#include <QDialog>

class LicenseManager;
class QLineEdit;
class QLabel;

// Диалог активации: ввод ключа + показ ID компьютера; вызывается при старте,
// если действующей лицензии нет.
class LicenseDialog : public QDialog {
    Q_OBJECT
public:
    explicit LicenseDialog(LicenseManager* manager, QWidget* parent = nullptr);

private:
    void OnActivate();
    LicenseManager* manager_;
    QLineEdit* key_edit_;
    QLineEdit* org_edit_;
    QLabel* status_;
};

#endif // LICENSEDIALOG_H
