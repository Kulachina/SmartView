QT += charts widgets testlib
CONFIG += console c++17 testcase
CONFIG -= app_bundle
TEMPLATE = app
TARGET = tst_chartoverview

INCLUDEPATH += $$PWD/../../src

SOURCES += \
    tst_chartoverview.cpp \
    $$PWD/../../src/chartoverview.cpp \
    $$PWD/../../src/data_base.cpp

HEADERS += \
    $$PWD/../../src/chartoverview.h \
    $$PWD/../../src/data_base.h \
    $$PWD/../../src/Data.h
