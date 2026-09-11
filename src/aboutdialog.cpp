#include "aboutdialog.h"
#include "version.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QPixmap>
#include <QFile>
#include <QFont>

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent){
    setWindowTitle("О программе");
    resize(560, 480);

    QLabel *icon = new QLabel();
    icon->setPixmap(QPixmap(":/SmartView.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QLabel *name = new QLabel("SmartView");
    QFont name_font = name->font();
    name_font.setPointSize(name_font.pointSize() + 6);
    name_font.setBold(true);
    name->setFont(name_font);
    QLabel *version = new QLabel("Версия " + QString(APP_VERSION));

    QVBoxLayout *title = new QVBoxLayout();
    title->addWidget(name);
    title->addWidget(version);

    // Растяжка в шапке только горизонтальная: вертикальная сделала бы всю строку
    // растягиваемой, и высоту забирала бы она, а не список изменений.
    QHBoxLayout *header = new QHBoxLayout();
    header->addWidget(icon, 0, Qt::AlignTop);
    header->addSpacing(12);
    header->addLayout(title);
    header->addStretch();

    QTextBrowser *changes = new QTextBrowser();
    changes->setOpenExternalLinks(true);
    changes->setMarkdown(ReadChangelog());

    QPushButton *close = new QPushButton("Закрыть");
    close->setDefault(true);
    connect(close, &QPushButton::clicked, this, &QDialog::accept);
    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(close);

    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->addLayout(header);
    vb->addWidget(changes, 1);
    vb->addLayout(buttons);
}

QString AboutDialog::ReadChangelog(){
    QFile file(":/CHANGELOG.md");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return "Список изменений недоступен.";
    }
    QString text = QString::fromUtf8(file.readAll());
    // Заголовок первого уровня нужен файлу в репозитории, а в окне он дублирует заголовок окна.
    if(text.startsWith("# ")){
        text = text.mid(text.indexOf('\n') + 1);
    }
    return text;
}
