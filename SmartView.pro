QT       += core gui charts core5compat network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/axiswindow.cpp \
    src/canalutils.cpp \
    src/chartview.cpp \
    src/checkpointswindow.cpp \
    src/createraport.cpp \
    src/deletesensorwindow.cpp \
    src/data_base.cpp \
    src/documentloader.cpp \
    src/dowland_file.cpp \
    src/error_table.cpp \
    src/las.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/masterpointswindow.cpp \
    src/sensorcanaleditor.cpp \
    src/serieswindow.cpp \
    src/updater.cpp \
    src/viewwindow.cpp

HEADERS += \
    src/Data.h \
    src/axiswindow.h \
    src/canalutils.h \
    src/chartview.h \
    src/checkpointswindow.h \
    src/createraport.h \
    src/deletesensorwindow.h \
    src/data_base.h \
    src/documentloader.h \
    src/dowland_file.h \
    src/error_table.h \
    src/las.h \
    src/mainwindow.h \
    src/masterpointswindow.h \
    src/sensorcanaleditor.h \
    src/serieswindow.h \
    src/updater.h \
    src/util.h \
    src/viewwindow.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources/resorce.qrc
