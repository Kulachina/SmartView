#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H
#pragma once
#include <QDialog>

// Окно «О программе»: название, версия и список изменений.
// Список берётся из CHANGELOG.md, зашитого в ресурсы, поэтому показывает изменения
// именно той сборки, которая установлена, и не требует сети.
class AboutDialog : public QDialog {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget* parent = nullptr);

private:
    static QString ReadChangelog();
};

#endif // ABOUTDIALOG_H
