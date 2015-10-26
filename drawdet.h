#ifndef DRAWDET
#define DRAWDET

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "cv.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;

#define VIDEO_WIDTH		640
#define VIDEO_HEIGHT	480

void findPoly();
void GetObjectHist(CvRect r);
void GetFourLines(CvSeq* lines);
void sortCorner(CvSeq* polyContour);
CvPoint transformPoint(const CvPoint pointToTransform, const CvMat* matrix);
void draw_processor(IplImage *frame, int *frame_num);

#endif // DRAWDET

