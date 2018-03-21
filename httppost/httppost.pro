#-------------------------------------------------
#
# Project created by QtCreator 2017-05-02T20:47:27
#
#-------------------------------------------------

QT       += core gui

QT += network
QT += multimedia
QT += script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = httppost
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
    MyGlobalShortCut/MyWinEventFilter.cpp \
    MyGlobalShortCut/MyGlobalShortCut.cpp \
    mvda_detection.cpp \
    mvoice_synthesis.cpp \
    mvoice_recognition.cpp

HEADERS  += mainwindow.h \
    MyGlobalShortCut/MyWinEventFilter.h \
    MyGlobalShortCut/MyGlobalShortCut.h \
    mvda_detection.h \
    mvoice_synthesis.h \
    mvoice_recognition.h

FORMS    += mainwindow.ui


RESOURCES += \
    image.qrc

INCLUDEPATH += $$PWD/D:/Program/opencv/include
DEPENDPATH += $$PWD/D:/Program/opencv/include
