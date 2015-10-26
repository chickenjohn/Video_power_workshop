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
