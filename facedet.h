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

#ifndef FACEDET_H
#define FACEDET_H
#define THDIRECX 3
#define THDIRECY 2
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/ml/ml.hpp"
#include <iostream>
#include <stdio.h>
#include <string>

using namespace cv;

void detface(Mat iplImg);

int DetectAndDraw(Mat img, CascadeClassifier& cascade, CascadeClassifier& ncascade);
int OTSU(Mat &img);
int* eyeShadowXY(Mat src);
int* noseShadowXY(IplImage* src);
void GetDirection(CvPoint Eye1,CvPoint Eye2,CvPoint Nose);

#endif // FACEDET_H
