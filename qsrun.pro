######################################################################
# Automatically generated by qmake (3.0) Mon Dec 25 15:21:45 2017
######################################################################

TEMPLATE = app
TARGET = qsrun
INCLUDEPATH += .

# Input
HEADERS += calculationengine.h \
           entryprovider.h \
           entrypushbutton.h \
           settingsprovider.h \
           singleinstanceserver.h \
           textoutputlabel.h \
           window.h
SOURCES += calculationengine.cpp \
           entryprovider.cpp \
           entrypushbutton.cpp \
           main.cpp \
           settingsprovider.cpp \
           singleinstanceserver.cpp \
           textoutputlabel.cpp \
           window.cpp
QT += widgets sql network
QT_CONFIG -= no-pkg-config
LIBS += -lcln
CONFIG += link_pkgconfig c++17
PKGCONFIG += libqalculate
