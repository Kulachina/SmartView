#include "licensedialog.h"
#include "licensemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QApplication>
#include <QClipboard>

LicenseDialog::LicenseDialog(LicenseManager* manager, QWidget* parent)
    : QDialog(parent), manager_(manager){
    setWindowTitle("Активация лицензии");
    setModal(true);
    QVBoxLayout* v = new QVBoxLayout(this);
    v->addWidget(new QLabel("Введите лицензионный ключ:"));
    key_edit_ = new QLineEdit();
    key_edit_->setPlaceholderText("XXXX-XXXX-XXXX-XXXX");
    v->addWidget(key_edit_);

    const QString fp = LicenseManager::MachineFingerprint();
    QLabel* fp_label = new QLabel("ID этого компьютера: " + fp.left(16) + "…");
    fp_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    v->addWidget(fp_label);
    QPushButton* copy_fp = new QPushButton("Скопировать ID компьютера");
    connect(copy_fp, &QPushButton::clicked, this, [fp](){ QApplication::clipboard()->setText(fp); });
    v->addWidget(copy_fp);

    status_ = new QLabel();
    status_->setStyleSheet("color: red;");
    status_->setWordWrap(true);
    v->addWidget(status_);

    QHBoxLayout* h = new QHBoxLayout();
    QPushButton* activate = new QPushButton("Активировать");
    QPushButton* quit = new QPushButton("Выход");
    h->addWidget(activate);
    h->addWidget(quit);
    v->addLayout(h);
    connect(activate, &QPushButton::clicked, this, &LicenseDialog::OnActivate);
    connect(quit, &QPushButton::clicked, this, &QDialog::reject);
}

void LicenseDialog::OnActivate(){
    status_->setStyleSheet("color: gray;");
    status_->setText("Активация…");
    QApplication::processEvents();
    const LicenseManager::Result r = manager_->Activate(key_edit_->text());
    if(r.ok){
        accept();
    } else {
        status_->setStyleSheet("color: red;");
        status_->setText(r.message);
    }
}
