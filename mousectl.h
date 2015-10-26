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

