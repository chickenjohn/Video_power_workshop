#-------------------------------------------------
#
# Project created by QtCreator 2015-06-15T19:20:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = video_power_workshop
TEMPLATE = app


SOURCES += main.cpp\
        maindialog.cpp \
    headctrl.cpp \
    drawer.cpp \
    facedet.cpp \
    mousectl.cpp

HEADERS  += maindialog.h \
    headctrl.h \
    drawer.h \
    facedet.h \
    mousectl.h

#LIBS += /usr/local/lib/libopencv_*

#for zybo:
INCLUDEPATH += /usr/local/zybo_opencv/include

LIBS += /usr/local/zybo_opencv/lib/*.so



