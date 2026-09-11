QT += charts widgets testlib core5compat
CONFIG += console c++17 testcase
CONFIG -= app_bundle
TEMPLATE = app
TARGET = tst_chartshift

INCLUDEPATH += $$PWD/../../src

SOURCES += \
    tst_chartshift.cpp \
    $$PWD/../../src/chartview.cpp \
    $$PWD/../../src/data_base.cpp

HEADERS += \
    $$PWD/../../src/chartview.h \
    $$PWD/../../src/data_base.h \
    $$PWD/../../src/Data.h
