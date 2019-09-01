######################################################################
# Automatically generated by qmake (3.0) Mon Dec 25 15:21:45 2017
######################################################################

TEMPLATE = app
TARGET = qsrun
INCLUDEPATH += .

# Input
HEADERS += config.h window.h \
    entrypushbutton.h \
    calculationengine.h \
    singleinstanceserver.h \
    configprovider.h
SOURCES += config.cpp main.cpp window.cpp \
    entrypushbutton.cpp \
    calculationengine.cpp \
    singleinstanceserver.cpp \
    configprovider.cpp
QT += widgets sql network
QT_CONFIG -= no-pkg-config
LIBS += -lcln
CONFIG += link_pkgconfig
PKGCONFIG += libqalculate
