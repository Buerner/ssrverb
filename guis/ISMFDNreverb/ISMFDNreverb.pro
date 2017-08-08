#-------------------------------------------------
#
# Project created by QtCreator 2016-11-25T16:09:13
#
#-------------------------------------------------

QT += core gui widgets

QT_CONFIG -= no-pkg-config

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7

QMAKE_CXXFLAGS += -std=c++11 -O3

TARGET = ISMFDNreverb
TEMPLATE = app

DESTDIR = build/
OBJECTS_DIR = build/.obj
MOC_DIR = build/.moc
RCC_DIR = build/.rcc
UI_DIR = build/.ui

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += jack sndfile fftw3f
}

INCLUDEPATH += ../../

QMAKE_LIBDIR += "$$PWD/"../../build/libs

LIBS += -lssrverb -laproque -lssrface
#LIBS +=  -laproque -lssrface -lreverbs

SOURCES += src/main.cpp\
           src/fdnwindow.cpp

HEADERS  += src/fdnwindow.h

FORMS    += src/fdnwindow.ui
