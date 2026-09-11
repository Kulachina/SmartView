QT += charts widgets testlib core5compat network xml
CONFIG += console c++17 testcase
CONFIG -= app_bundle
TEMPLATE = app
TARGET = tst_svformat

INCLUDEPATH += $$PWD/../../src

SOURCES += \
    tst_svformat.cpp \
    $$PWD/../../src/dowland_file.cpp \
    $$PWD/../../src/data_base.cpp \
    $$PWD/../../src/canalutils.cpp

HEADERS += \
    $$PWD/../../src/dowland_file.h \
    $$PWD/../../src/data_base.h \
    $$PWD/../../src/canalutils.h \
    $$PWD/../../src/Data.h
