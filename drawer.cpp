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

#include "drawer.h"

using namespace cv;
using namespace std;

const CvPoint2D32f canvas[4] = { cvPoint2D32f(400, 0), cvPoint2D32f(0, 0), cvPoint2D32f(400, 300), cvPoint2D32f(0, 300) };

bool paused = false;
bool paper_ready = true;
bool pen_ready = false;
bool paper_done = false;
bool object_found = false;
bool enMediaBlur = true;
bool outXY = false;
bool isWritting = false;
bool warpEn = false;

CvHistogram *hist = 0;
CvBox2D track_box;
CvConnectedComp track_comp;

int hdims = 16;
float hranges_arr[] = { 0, 180 };
float* hranges = hranges_arr;
int frame_num=0;

CvMat* warp_mat = cvCreateMat(3, 3, CV_32FC1); // 透视变换矩阵
CvPoint2D32f corners[4];
CvPoint pos_hold = cvPoint(0, 0);	// 用于判断笔尖是否移动
CvPoint pos_pre = cvPoint(-1, -1);	// 存储笔尖上一帧的位置
CvPoint pos_now = cvPoint(-1, -1);	// 存储笔尖当前帧的位置
CvPoint warpPos = cvPoint(-1, -1); // 变换后的点
bool isQuad = false;				// 判断纸张轮廓是否为四边形

CvRect r;
CvRect track_window;

IplImage *img_hsv = 0, *mask_pen = 0, *mask_paper = 0, *paper_contour = 0, *pen_contour = 0, *img_gray = 0;
IplImage *img_hue = 0, *color_dst = 0, *backproject = 0, *mask = 0;
IplImage *myCanvas = 0, *warpCanvas = 0;
IplImage *canvas_x2 = 0;
bool isTwice = false;

int line_width=10;
int colorsel=0;
QColor finalcolor;
QLabel *paintarea;
QLabel * number_rec;

Drawer::Drawer(QWidget *parent) : QWidget(parent)
{
    createToolBar();

    connect(dra_on_btn,SIGNAL(clicked(bool)),this,SLOT(drawerstart()));
    connect(dra_off_btn,SIGNAL(clicked(bool)),this,SLOT(enddraw()));
    connect(clearBtn,SIGNAL(clicked(bool)),this,SLOT(clean()));
    connect(widthSpinBox,SIGNAL(valueChanged(int)),this,SLOT(changewidth(int)));
}

void Drawer::createToolBar()
{
    paint = new QPixmap(640,480);
    paint->fill(Qt::white);
    paintarea = new QLabel;
    widthSpinBox = new QSpinBox;
    widthSpinBox->setValue(10);
    widthSpinBox->setMaximumHeight(50);

    colorBtn = new QPushButton;
    QPixmap pixmap(100,100);
    pixmap.fill(Qt::black);
    colorBtn->setMinimumSize(100,100);
    colorBtn->setIcon(QIcon(pixmap));
    connect(colorBtn,SIGNAL(clicked()),this,SLOT(ShowColor()));

    clearBtn = new QPushButton;
    clearBtn ->setText(tr("clean"));
    clearBtn->setMinimumSize(100,100);

    dra_on_btn = new QPushButton(tr("Start Drawing"));
    dra_off_btn =  new QPushButton(tr("Stop Drawing"));
    dra_on_btn->setMinimumSize(100,100);
    dra_off_btn->setMinimumSize(100,100);

    paintarea->setPixmap(*paint);

    QString num(tr("welcome!"));
    number_rec = new QLabel;
    number_rec->setText(num);
    number_rec->setFixedHeight(30);

    //layout
    dra_box_layout = new QVBoxLayout;
    dra_paint_layout = new QGridLayout;
    dra_layout = new QGridLayout;
    dra_box_layout->addWidget(widthSpinBox);
    dra_box_layout->addWidget(colorBtn);
    dra_box_layout->addWidget(clearBtn);
    dra_box_layout->addWidget(dra_on_btn);
    dra_box_layout->addWidget(dra_off_btn);
    dra_box_layout->setSpacing(20);
    dra_paint_layout->addWidget(paintarea,0,0);
    dra_paint_layout->addWidget(number_rec,1,0);

    dra_paint_layout->setRowStretch(0,20);
    dra_paint_layout->setRowStretch(1,1);
    dra_layout->addLayout(dra_paint_layout,0,0);
    dra_layout->addLayout(dra_box_layout,0,1);
    dra_layout->setColumnStretch(0,4);
    dra_layout->setColumnStretch(1,1);
    dra_layout->setSpacing(20);
    setLayout(dra_layout);

}

void Drawer::ShowColor()
{
    //offer three type of colors
    if(colorsel<1)
        colorsel++;
    else
        colorsel=0;
    QColor color;
    switch(colorsel)
    {
    case 0 :
        color = QColor(Qt::black);
        break;
    case 1 :
        color = QColor(Qt::green);
        break;
    }

    if(color.isValid())
    {
        finalcolor = color;
        QPixmap p(100,100);
        p.fill(color);
        colorBtn->setIcon(QIcon(p));
    }
}

void startdraw()
{
    extern IplImage *frame;
    if (!img_hsv)
    {
        img_hsv = cvCreateImage(cvGetSize(frame), 8, 3);
        mask_pen = cvCreateImage(cvGetSize(frame), 8, 1);
        mask_paper = cvCreateImage(cvGetSize(frame), 8, 1);
        mask = cvCreateImage(cvGetSize(frame), 8, 1);
        paper_contour = cvCreateImage(cvGetSize(frame), 8, 1);
        //pen_contour = cvCreateImage(cvGetSize(frame), 8, 3);
        img_hue = cvCreateImage(cvGetSize(frame), 8, 1);
        backproject = cvCreateImage(cvGetSize(frame), 8, 1);
        color_dst = cvCreateImage(cvGetSize(frame), 8, 3);
        //paper_line = cvCreateImage(cvGetSize(frame), 8, 3);
        //myCanvas = cvCreateImage(cvGetSize(frame), 8, 3);
        warpCanvas = cvCreateImage(cvSize(400,300), 8, 3);
        canvas_x2 = cvCreateImage(cvSize(PAPER_WIDTH * 2, PAPER_HEIGHT * 2), 8, 3);
        cvSet(warpCanvas, cvScalar(255, 255, 255));
        cvSet(canvas_x2, cvScalar(255, 255, 255));
        hist = cvCreateHist(1, &hdims, CV_HIST_ARRAY, &hranges, 1);
    }
    cvCvtColor(frame, img_hsv, CV_BGR2HSV);

    // 如果还没识别出笔，则探测笔的位置
    if (!object_found)
    {
        // 如果已经得到纸张的区域
        if (paper_done)
        {
            cvSmooth(frame, frame, CV_GAUSSIAN, 3, 0);
            cvInRangeS(img_hsv, Scalar(100, 43, 46), Scalar(125, 256, 256), mask_pen); // 获取图像中蓝色部分作为笔的掩模
            cvSmooth(mask_pen, mask_pen, CV_GAUSSIAN, 5, 0);
            cvAnd(mask_pen, mask_paper,mask_pen);
            //cvZero(pen_contour);
            findPoly();	// 找出最大连通域，则可判断为笔
            //cvAdd(frame, pen_contour, frame);
        }
    }
    // 如果已经识别到笔，则进行跟踪
    else
    {
        cvInRangeS(img_hsv, Scalar(100, 43, 46), Scalar(125, 256, 256), mask);
        cvSplit(img_hsv, img_hue, 0, 0, 0);
        cvCalcBackProject(&img_hue, backproject, hist);
        cvAnd(mask, mask_paper, mask, 0);
        cvAnd(backproject, mask, backproject, 0);
        cvCamShift(backproject, track_window,
                   cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1),
                   &track_comp, &track_box);
        track_window.x = track_comp.rect.x - 16;
        track_window.y = track_comp.rect.y - 16;
        track_window.width = track_comp.rect.width  + 32;
        track_window.height = track_comp.rect.height + 32;
        // 若跟踪窗口太大或太小则判断为跟丢
        cout<<"width=\n"<<track_comp.rect.width<<endl;
        cout<<"height=\n"<<track_comp.rect.height<<endl;
        //if ((track_comp.rect.width < 16 && track_comp.rect.height < 16) || (track_comp.rect.width > 200 || track_comp.rect.height > 200))
        if(isLost())
        {
            printf("lost!");
            object_found = false;
            isWritting = false;
        }
        else
        {
            cvZero(color_dst);
            if (outXY)
                cout << track_box.center.x << ", " << track_box.center.y << endl;
            if (frame_num == 0)
            {
                pos_hold.x = track_box.center.x;
                pos_hold.y = track_box.center.y;
            }
            if (pos_hold.x > track_box.center.x - 3 && pos_hold.x < track_box.center.x + 3)
                if (pos_hold.y > track_box.center.y - 3 && pos_hold.y < track_box.center.y + 3)
                    frame_num++;
                else
                    frame_num = 0;
            else
                frame_num = 0;
            // 如果笔保持20帧不动则判断为起笔/落笔
            if (frame_num == 14)
            {
                isWritting = !isWritting;
                if (isWritting == false)
                {
                    pos_pre.x = -1;
                }
                cout << isWritting << endl;
            }
            if (isWritting)
            {
                if (pos_pre.x == -1)
                {
                    pos_pre.x = track_box.center.x;
                    pos_pre.y = track_box.center.y;
                }
                else
                {
                    pos_pre.x = pos_now.x;
                    pos_pre.y = pos_now.y;
                }
                pos_now.x = track_box.center.x;
                pos_now.y = track_box.center.y;
                pos_now.x = 0.5 * (double)pos_now.x + 0.5 * (double)pos_pre.x;
                pos_now.y = 0.5 * (double)pos_now.y + 0.5 * (double)pos_pre.y;
                //warpPos.x = CV_MAT_ELEM(*warp_mat, float, 0, 0)*pos_now.x;
                CvPoint pt1 = transformPoint(pos_pre, warp_mat);
                CvPoint pt2 = transformPoint(pos_now, warp_mat);
                cvLine(warpCanvas,pt1,pt2,CV_RGB(finalcolor.red(),finalcolor.green(),finalcolor.blue()), line_width, 8, 0);
                if (isTwice)
                    cvLine(canvas_x2, cvPoint(pt1.x*2,pt1.y*2), cvPoint(pt2.x*2,pt2.y*2), cvScalarAll(0), 2, 8, 0);
                //cvWarpPerspective(myCanvas, warpCanvas, warp_mat);
            }
            // 画出跟踪框
            //cvEllipseBox(color_dst, track_box, CV_RGB(255, 0, 0), 3, CV_AA, 0);
            //cvRectangle(color_dst, cvPoint(track_window.x, track_window.y), cvPoint(track_window.x + track_window.width, track_window.y + track_window.height), CV_RGB(0, 0, 255), 4, 8, 0);
        }
    }
    // 如果纸张摆放正确，则进行纸张的识别
    if (paper_ready)
    {
        paper_ready = false;
        cvInRangeS(img_hsv, Scalar(0, 0, 46), Scalar(180, 43, 255), mask_paper);
        cvSmooth(mask_paper, mask_paper, CV_MEDIAN);
        cvErode(mask_paper, mask_paper, cvCreateStructuringElementEx(5, 5, 0, 0, CV_SHAPE_ELLIPSE));
        cvDilate(mask_paper, mask_paper, cvCreateStructuringElementEx(5, 5, 0, 0, CV_SHAPE_ELLIPSE));
        // 寻找最大连通域，判断为纸张

        CvSeq* contour = 0;
        CvMemStorage* storage = cvCreateMemStorage(0);
        cvFindContours(mask_paper, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        CvSeq* _contour = contour;
        double maxarea = 0;
        double minarea = 3;
        int n = -1, m = 0;//n为面积最大轮廓索引，m为迭代索引
        for (; contour != 0; contour = contour->h_next)
        {

            double tmparea = fabs(cvContourArea(contour));
            if (tmparea < minarea)
            {
                cvSeqRemove(contour, 0); //删除面积小于设定值的轮廓
                continue;
            }
            CvRect aRect = cvBoundingRect(contour, 0);
            if ((aRect.width / aRect.height) < 1)
            {
                cvSeqRemove(contour, 0); //删除宽高比例小于设定值的轮廓
                continue;
            }
            if (tmparea > maxarea)
            {
                maxarea = tmparea;
                n = m;
            }
            m++;
        }
        contour = _contour;
        int count = 0;
        for (; contour != 0; contour = contour->h_next)
        {
            count++;
            double tmparea = fabs(cvContourArea(contour));
            if (tmparea == maxarea)
            {
                CvScalar color = CV_RGB(255, 255, 255);
                CvSeq* polyCont;
                polyCont = cvApproxPoly(contour, sizeof(CvContour), storage, CV_POLY_APPROX_DP, cvContourPerimeter(contour)*0.05, 0);
                cvZero(mask_paper);
                cvDrawContours(mask_paper, polyCont, cvScalarAll(255), cvScalarAll(255), -1, CV_FILLED);
                cout << polyCont->total << endl;
                if (polyCont->total == 4)
                {
                    isQuad = true;
                    sortCorner(polyCont);
                    for (int k = 0; k < 4; k++)
                    {
                        CvPoint* corner = (CvPoint*)cvGetSeqElem(polyCont, k);
                        corners[k] = cvPointTo32f(corner[0]);
                        cvCircle(myCanvas, corner[0], 3, cvScalar(255, 0, 0), -1, 8, 0);
                    }
                    cvGetPerspectiveTransform(corners, canvas, warp_mat);
                    //cvWarpPerspective(myCanvas, warpCanvas, warp_mat);
                    paper_done = true;	// 纸张识别完成
                }
                else
                    isQuad = false;
                break;
            }
        }
    }
    // 显示跟踪框
    if (object_found)
        cvAdd(frame, color_dst, frame);
    if (warpEn)
    {
        cvWarpPerspective(myCanvas, warpCanvas, warp_mat);
        warpEn = false;
    }

    //cvShowImage("Warp Canvas", warpCanvas);
    cvCvtColor(warpCanvas , warpCanvas, CV_BGR2RGB);

    QImage image((const uchar*)warpCanvas->imageData,warpCanvas->width,warpCanvas->height,QImage::Format_RGB888);
    paintarea->setPixmap(QPixmap::fromImage(image));

}

void Drawer::enddraw()
{
    pos_hold = cvPoint(0, 0);	// 用于判断笔尖是否移动
    pos_pre = cvPoint(-1, -1);	// 存储笔尖上一帧的位置
    pos_now = cvPoint(-1, -1);	// 存储笔尖当前帧的位置
    warpPos = cvPoint(-1, -1); // 变换后的点
    paused = false;
    paper_ready = true;
    pen_ready = false;
    object_found = false;
    enMediaBlur = true;
    outXY = false;
    isWritting = false;
    warpEn = false;
}

void findPoly()
{
    // 连通域

    CvSeq* contour = 0;
    CvMemStorage* storage = cvCreateMemStorage(0);
    cvFindContours(mask_pen, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
    CvSeq* _contour = contour;
    double maxarea = 0;
    double minarea = 60;
    int n = -1, m = 0;//n为面积最大轮廓索引，m为迭代索引
    for (; contour != 0; contour = contour->h_next)
    {

        double tmparea = fabs(cvContourArea(contour));
        if (tmparea < minarea)
        {
            cvSeqRemove(contour, 0); //删除面积小于设定值的轮廓
            continue;
        }
        CvRect aRect = cvBoundingRect(contour, 0);
        if (((double)aRect.width / (double)aRect.height) > 1.4)
        {
            cvSeqRemove(contour, 0); //删除宽高比例小于设定值的轮廓
            continue;
        }
        if (tmparea > maxarea)
        {
            maxarea = tmparea;
            n = m;
        }
        m++;
    }
    contour = _contour;
    int count = 0;
    for (; contour != 0; contour = contour->h_next)
    {
        if (maxarea == 0)
            break;
        count++;
        double tmparea = fabs(cvContourArea(contour));

        if (tmparea == maxarea)
        {
            CvScalar color = CV_RGB(255, 0, 0);
            //cvZero(pen_contour);
            //cvDrawContours(pen_contour, contour, color, color, -1, 1, 8);
            GetObjectHist(cvBoundingRect(contour, 0));
            object_found = true;
            pen_ready = false;
        }
    }
}

// 计算笔的直方图
void  GetObjectHist(CvRect r)
{
    cvSplit(img_hsv, img_hue, 0, 0, 0);
    cvInRangeS(img_hsv, cvScalar(0, 30, 10, 0), cvScalar(180, 256, 256, 0), mask);
    //cvRectangle(pen_contour, cvPoint(r.x, r.y), cvPoint(r.x + r.width, r.y + r.height), cvScalar(180, 255, 255), 3, 8, 0);
    cvSetImageROI(img_hue, r);
    cvSetImageROI(mask, r);
    cvCalcHist(&img_hue, hist, 0, mask);
    cvResetImageROI(img_hue);
    cvResetImageROI(mask);
    track_window = r;
}

// 排序四边形轮廓顶点
void  sortCorner(CvSeq* polyContour)
{
    CvPoint* ptr = 0;
    CvPoint center = cvPoint(0, 0);
    for (int i = 0; i < 4; i++)
    {
        ptr = (CvPoint*)cvGetSeqElem(polyContour, i);
        center.x += ptr[0].x;
        center.y += ptr[0].y;
    }
    center.x *= 0.25;
    center.y *= 0.25;

    CvPoint sortedCorner[4];
    // Top-Left Top-Right Bottom-Left Bottom-Right
    for (int i = 0; i < 4; i++)
    {
        ptr = (CvPoint*)cvGetSeqElem(polyContour, i);
        if (ptr[0].y > center.y) // before: 1 0 3 2
            if (ptr[0].x > center.x)
                sortedCorner[2] = ptr[0];
            else
                sortedCorner[3] = ptr[0];
        else
            if (ptr[0].x > center.x)
                sortedCorner[0] = ptr[0];
            else
                sortedCorner[1] = ptr[0];
    }
    for (int i = 0; i < 4; i++)
    {
        ptr = (CvPoint*)cvGetSeqElem(polyContour, i);
        ptr[0] = sortedCorner[i];
    }
}

CvPoint  transformPoint(const CvPoint pointToTransform, const CvMat* matrix)
{
    float coordinates[3] = { pointToTransform.x, pointToTransform.y, 1 };
    CvMat originVector = cvMat(3, 1, CV_32FC1, coordinates);
    CvMat transformedVector = cvMat(3, 1, CV_32FC1, coordinates);
    cvMatMul(matrix, &originVector, &transformedVector);
    CvPoint outputPoint = cvPoint((int)(cvmGet(&transformedVector, 0, 0) / cvmGet(&transformedVector, 2, 0)), (int)(cvmGet(&transformedVector, 1, 0) / cvmGet(&transformedVector, 2, 0)));
    return outputPoint;
}

void Drawer::clean()
{
    paint->fill(Qt::white);
    paintarea->setPixmap(*paint);
    memset((unsigned char*)warpCanvas->imageData,255,warpCanvas->imageSize);
}

void Drawer::changewidth(int w)
{
    line_width = w;
}

void Drawer::drawerstart()
{
    extern int state;
    state = 2;
}

// 文字放大，画纸缩小
void zoomIn(CvPoint pt)
{
    extern bool isTwice;
    if(isTwice)
    {
        extern IplImage * warpCanvas;
        //transformPoint(pt_before_trans,warp_mat);
        CvRect r;
        if (pt.x > 100 && pt.x < 300 && pt.y > 75 && pt.y < 225)
            r = cvRect(pt.x * 2 - 200, pt.y * 2 - 150, PAPER_WIDTH, PAPER_HEIGHT);
        else if (pt.x <= 100 && pt.y <= 75)
            r = cvRect(pt.x, pt.y, PAPER_WIDTH, PAPER_HEIGHT);
        else if (pt.x <= 100 && pt.y >= 225)
            r = cvRect(pt.x, pt.y - 150, PAPER_WIDTH, PAPER_HEIGHT);
        else if (pt.x >= 300 && pt.y <= 75)
            r = cvRect(pt.x - 200, pt.y, PAPER_WIDTH, PAPER_HEIGHT);
        else if (pt.x >= 300 && pt.y >= 225)
            r = cvRect(pt.x - 200, pt.y - 225, PAPER_WIDTH, PAPER_HEIGHT);
        isTwice = false;
        cvSetImageROI(canvas_x2, r);
        cvCopy(canvas_x2, warpCanvas);
        //cvShowImage("Warp Canvas", warpCanvas);
        cvCvtColor(warpCanvas , warpCanvas, CV_BGR2RGB);
        QImage image((const uchar*)warpCanvas->imageData,warpCanvas->width,warpCanvas->height,QImage::Format_RGB888);
        paintarea->setPixmap(QPixmap::fromImage(image));
    }
}
// 文字缩小，图纸扩大
void zoomOut(CvPoint pt)
{
    extern bool isTwice;
    if(!isTwice)
    {
        extern IplImage * warpCanvas;
        CvRect  r = cvRect(pt.x, pt.y, PAPER_WIDTH, PAPER_HEIGHT);
        cvSetImageROI(canvas_x2, r);
        cvCopy(warpCanvas, canvas_x2);
        cvResetImageROI(canvas_x2);
        cvResize(canvas_x2, warpCanvas, CV_INTER_CUBIC);
        //cvShowImage("Warp Canvas", warpCanvas);
        isTwice = true;
        cvCvtColor(warpCanvas , warpCanvas, CV_BGR2RGB);
        QImage image((const uchar*)warpCanvas->imageData,warpCanvas->width,warpCanvas->height,QImage::Format_RGB888);
        paintarea->setPixmap(QPixmap::fromImage(image));
    }
}

bool isLost()
{
    CvSeq* contour = 0;
    CvMemStorage* storage = cvCreateMemStorage(0);
    cvFindContours(backproject, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    double minarea = 100;
    for (; contour != 0; contour = contour->h_next)
    {
        double tmparea = fabs(cvContourArea(contour));
        if (tmparea > minarea)
            return false;
    }
    return true;
}
