#-----------------------------------------------------------------------------------------------------------------
#
# Project created by QtCreator 2015-06-15T19:20:20
#
# Copyright (C) 2015 chickenjohn
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Attached below is the contact of chickenjohn:
# Email: chickenjohn93@outlook.com
#-----------------------------------------------------------------------------------------------------------------

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



