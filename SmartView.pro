QT       += core gui charts core5compat

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chartview.cpp \
    createraport.cpp \
    data_base.cpp \
    dowland_file.cpp \
    error_table.cpp \
    las.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Data.h \
    chartview.h \
    createraport.h \
    data_base.h \
    dowland_file.h \
    error_table.h \
    las.h \
    mainwindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resorce.qrc
