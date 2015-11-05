//------------------------------------------------------------------------------------------------------------------
//Copyright (C) 2015 chickenjohn

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

// Attached below is the contact of chickenjohn:
// Email: chickenjohn93@outlook.com
//------------------------------------------------------------------------------------------------------------------

#ifndef HEADCTRL_H
#define HEADCTRL_H
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <QDialog>
#include <QHBoxLayout>
#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QTimer>
#include <QLabel>
#include <QGridLayout>
#include <QPainter>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include "facedet.h"


using namespace cv;

class Headctrl : public QWidget
{
    Q_OBJECT
public:
    explicit Headctrl(QWidget *parent = 0);


signals:

private slots:
    void headctrlstart();
    void headctrlend();

private:
    QPushButton *ctl_on_btn;
    QPushButton *ctl_off_btn;
    QGridLayout *ctl_layout;
    QVBoxLayout *ctl_sublayout;

    IplImage *handleimg;
    QTimer *timer;
};

void getframe();

#endif // HEADCTRL_H
