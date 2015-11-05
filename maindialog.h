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

#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <QWidget>
#include <QTabWidget>
#include <QTimer>
#include <QPixmap>
#include <QLabel>
#include "headctrl.h"
#include "drawer.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <QTimer>
#include "mousectl.h"

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = 0);
    ~MainDialog();

public slots:
    void startcapture();
    void pagechange(int index);
    void changeclick();

signals:
   void draw_end_sig(void);

private:
    QTabWidget *tab;
    QLabel * state_label;
    Headctrl *headctrl;
    Drawer *drawer;
};



#endif // MAINDIALOG_H
