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

#ifndef MOUSECTL
#define MOUSECTL
#include <stdio.h>
#include <linux/input.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include "opencv/cv.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <QTimer>
#include "drawer.h"

using namespace std;
using namespace cv;

void mouse_ctrl(void);
int initdirect();
void getdirect(int *rel_x, int *rel_y);
int detfinger(int *x, int *y);
void simulate_mouse(int fd, int rel_x, int rel_y);
void simulate_key(int fd, int kval);
int distance2(Point a,Point b);
int getgesture(void);
bool Kcompare();






#endif // MOUSECTL

