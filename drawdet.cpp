#include "drawdet.h"

using namespace cv;
using namespace std;

IplImage *img_hsv = 0, *mask_pen = 0, *mask_paper = 0, *paper_contour = 0, *pen_contour = 0, *img_gray = 0;
IplImage *img_hue = 0, *color_dst = 0, *backproject = 0, *mask = 0;
IplImage *myCanvas = 0, *warpCanvas = 0;

const CvPoint2D32f canvas[4] = { cvPoint2D32f(400, 0), cvPoint2D32f(0, 0), cvPoint2D32f(400, 300), cvPoint2D32f(0, 300) };

bool paused = false;
bool paper_ready = false;
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

CvRect r;
CvRect track_window;

IplImage * draw_processor(IplImage *frame, int *frame_num)
{
    CvMat* warp_mat = cvCreateMat(3, 3, CV_32FC1); // 透视变换矩阵
    CvPoint2D32f corners[4];
    CvPoint pos_hold = cvPoint(0, 0);	// 用于判断笔尖是否移动
    CvPoint pos_pre = cvPoint(-1, -1);	// 存储笔尖上一帧的位置
    CvPoint pos_now = cvPoint(-1, -1);	// 存储笔尖当前帧的位置
    CvPoint warpPos = cvPoint(-1, -1); // 变换后的点
    bool isQuad = false;				// 判断纸张轮廓是否为四边形

        /*
            如果图像为空则进行初始化
        */
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
            cvSet(myCanvas, cvScalar(255, 255, 255));
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
                cvAnd(mask_pen, mask_paper, mask_pen);
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
            track_window = track_comp.rect;
            // 若跟踪窗口太大或太小则判断为跟丢
            if ((track_window.width < 3 && track_window.height < 3) || (track_window.width > 200 || track_window.height > 200))
                object_found = false;
            else
            {
                cvZero(color_dst);
                if (outXY)
                    cout << track_box.center.x << ", " << track_box.center.y << endl;
                if (*frame_num == 0)
                {
                    pos_hold.x = track_box.center.x;
                    pos_hold.y = track_box.center.y;
                }
                if (pos_hold.x > track_box.center.x - 3 && pos_hold.x < track_box.center.x + 3)
                    if (pos_hold.y > track_box.center.y - 3 && pos_hold.y < track_box.center.y + 3)
                        (*frame_num)++;
                    else
                        (*frame_num) = 0;
                else
                    (*frame_num) = 0;
                // 如果笔保持20帧不动则判断为起笔/落笔
                if ((*frame_num) == 14)
                {
                    isWritting = !isWritting;
                    if (isWritting == false)
                        pos_pre.x = -1;
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
                    pos_now.x = 0.3 * (double)pos_now.x + 0.7 * (double)pos_pre.x;
                    pos_now.y = 0.3 * (double)pos_now.y + 0.7 * (double)pos_pre.y;
                    //warpPos.x = CV_MAT_ELEM(*warp_mat, float, 0, 0)*pos_now.x;

                    cvLine(warpCanvas, transformPoint(pos_pre, warp_mat), transformPoint(pos_now, warp_mat) , cvScalarAll(0), 1, 8, 0);
                    //cvWarpPerspective(myCanvas, warpCanvas, warp_mat);
                }
                // 画出跟踪框
                cvEllipseBox(color_dst, track_box, CV_RGB(255, 0, 0), 3, CV_AA, 0);
                cvRectangle(color_dst, cvPoint(track_window.x, track_window.y), cvPoint(track_window.x + track_window.width, track_window.y + track_window.height), CV_RGB(0, 0, 255), 4, 8, 0);
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
                        cvWarpPerspective(myCanvas, warpCanvas, warp_mat);
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

        //cvShowImage("Source Video", frame);
        //cvShowImage("Canvas", myCanvas);
        cvShowImage("Warp Canvas", warpCanvas);
        char c = waitKey(12);
        switch (c)
        {
        case ' ':
            paper_ready = true;
            break;
        case 'p':
            pen_ready = true;
            break;
        case 'r':
            pen_ready = true;
            object_found = false;
            break;
        case 'q':
            outXY = !outXY;
            break;
        case 'w':
            warpEn = true;
            break;
        }

    }
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
void GetObjectHist(CvRect r)
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
void sortCorner(CvSeq* polyContour)
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
        if (ptr[0].y > center.y)
            if (ptr[0].x > center.x)
                sortedCorner[1] = ptr[0];
            else
                sortedCorner[0] = ptr[0];
        else
            if (ptr[0].x > center.x)
                sortedCorner[3] = ptr[0];
            else
                sortedCorner[2] = ptr[0];
    }
    for (int i = 0; i < 4; i++)
    {
        ptr = (CvPoint*)cvGetSeqElem(polyContour, i);
        ptr[0] = sortedCorner[i];
    }
}

CvPoint transformPoint(const CvPoint pointToTransform, const CvMat* matrix)
{
    double coordinates[3] = { pointToTransform.x, pointToTransform.y, 1 };
    CvMat originVector = cvMat(3, 1, CV_64F, coordinates);
    CvMat transformedVector = cvMat(3, 1, CV_64F, coordinates);
    cvMatMul(matrix, &originVector, &transformedVector);
    CvPoint outputPoint = cvPoint((int)(cvmGet(&transformedVector, 0, 0) / cvmGet(&transformedVector, 2, 0)), (int)(cvmGet(&transformedVector, 1, 0) / cvmGet(&transformedVector, 2, 0)));
    return outputPoint;
}
