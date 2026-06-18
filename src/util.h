#ifndef UTIL_H
#define UTIL_H
#pragma once
#include <QtGlobal>
#include <QTabWidget>
#include <QWidget>

// Округление времени в миллисекундах до целых секунд.
inline qint64 RoundToSec(qint64 ms){
    return ((ms + 500) / 1000) * 1000;
}

// Удаляет все вкладки QTabWidget вместе с их виджетами.
inline void ClearTabWidget(QTabWidget* tab){
    while (tab->count() > 0){
        QWidget* page = tab->widget(0);
        tab->removeTab(0);
        page->deleteLater();
    }
}

#endif // UTIL_H
