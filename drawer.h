#ifndef DRAWER_H
#define DRAWER_H

#include <QWidget>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTextBrowser>
#include <QTimer>
#include <QDebug>
#include  <QToolBar>
#include <QColorDialog>
#include <QString>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv/cv.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "mousectl.h"

using namespace cv;
using namespace std;

#define VIDEO_WIDTH		640
#define VIDEO_HEIGHT	480
#define PAPER_WIDTH		400
#define PAPER_HEIGHT	300



class Drawer : public QWidget
{
    Q_OBJECT
public:
    explicit Drawer(QWidget *parent = 0);
    void createToolBar();

public slots:
    void ShowColor();
    void enddraw();
    void drawerstart();
    void clean();
    void changewidth(int w);


private:
    QLabel *styleLabel;
    QComboBox *styleComboBox;
    QSpinBox *widthSpinBox;
    QPushButton *colorBtn;
    QPushButton *clearBtn;
    QPushButton *dra_on_btn;
    QPushButton *dra_off_btn;
    QVBoxLayout *dra_box_layout;
    QGridLayout  *dra_paint_layout;
    QGridLayout  *dra_layout;

    QPixmap *paint;


};

void startdraw();
void findPoly();
void GetObjectHist(CvRect r);
void GetFourLines(CvSeq* lines);
void sortCorner(CvSeq* polyContour);
CvPoint transformPoint(const CvPoint pointToTransform, const CvMat* matrix);
void zoomIn(CvPoint pt);
void zoomOut(CvPoint pt);
bool isLost(void);


#endif // DRAWER_H
