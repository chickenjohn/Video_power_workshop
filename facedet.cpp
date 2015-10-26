#include "facedet.h"

using namespace std;
using namespace cv;

#define THDIRECX 3
#define THDIRECY 2


int otsu;
int otsu_cnt;
CvPoint last_Leye,last_Reye,last_nose;
CvPoint Leye,Reye,nose;
CvPoint Face;
CvRect default_rc;
Mat img=Mat::zeros(Size(640,480),CV_8UC3);
Mat HandImg = Mat::zeros(Size(320,240),CV_8UC3);
bool start=false;

void detface(Mat iplImg)
{
    int k=0;
    otsu=0;
    otsu_cnt=5;
    double t=0;
    t = (double)cvGetTickCount();
    extern CascadeClassifier cascade,ncascade;//创建级联分类器对象

    default_rc.x=40;
    default_rc.y=0;
    default_rc.width=240;
    default_rc.height=240;

    resize(iplImg, HandImg,Size(320,240));
    k = DetectAndDraw(HandImg, cascade,ncascade);

    if(!k)
    {
        if(!start)
        {
            last_Leye=Leye;
            last_Reye=Reye;
            last_nose=nose;
            start=true;
        }
        else
        {
            Leye.x=Leye.x*0.5+last_Leye.x*0.5;Leye.y=Leye.y*0.5+last_Leye.y*0.5;
            Reye.x=Reye.x*0.5+last_Reye.x*0.5;Leye.y=Reye.y*0.5+last_Reye.y*0.5;
            nose.x=nose.x*0.5+last_nose.x*0.5;nose.y=nose.y*0.5+last_nose.y*0.5;
            last_Leye=Leye;
            last_Reye=Reye;
            last_nose=nose;
        }
        circle(HandImg,Leye,2,CV_RGB(255,255,0),2);
        circle(HandImg,Reye,2,CV_RGB(255,255,0),2);
        circle(HandImg,nose,2,CV_RGB(255,255,0),2);
        circle(HandImg,Face,2,CV_RGB(0,255,255),3);

        GetDirection(Leye,Reye,nose);

        if(otsu_cnt<10)otsu_cnt+=1;
        else if(otsu_cnt==10)otsu_cnt=0;
    }
    else {start=false;otsu_cnt=10;}
    //cvShowImage("test",HandImg);
    t = (double)cvGetTickCount() - t;
    system("clear");
    printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000));

}

int DetectAndDraw(Mat img, CascadeClassifier& cascade, CascadeClassifier& ncascade)
{
    int* pxy;

    CvPoint p1,p2;    //人脸区域矩形对角点
    CvRect faceRoi;
    CvPoint p3,p4;    //人眼
    CvRect LeyeRoi,ReyeRoi,eye;
    CvPoint p5,p6;    //鼻子
    CvRect noseRoi;


    int i = 0;
    Point center;
    vector<Rect> faces;

    //cvSmooth(img,img,CV_BLUR,3,3,0,0);          //均值滤波

    Mat fgray ;
    cvtColor( img, fgray, CV_BGR2GRAY );
    equalizeHist( fgray, fgray );

    //cvSetImageROI(gray,default_rc);
    cascade.detectMultiScale( fgray , faces,
                              1.4, 3,
                              CV_HAAR_FIND_BIGGEST_OBJECT
                              |CV_HAAR_DO_ROUGH_SEARCH
                              |CV_HAAR_SCALE_IMAGE
                              ,
                              Size(100, 100) );
    //cvResetImageROI(gray);

    //faceRoi.x=0;faceRoi.y=0;
    //faceRoi.width=320;faceRoi.height=240;

    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++)
    {
        //Scalar color = colors[i%8];

        ///////////////////////人脸矩形//////////////////////////
        //CvPoint p1,p2;

        Face.x = r->x+r->width*0.5;
        Face.y = r->y+r->height*0.5;

        p1.x=r->x;                      //ROI偏移量
        p1.y=r->y;
        p2.x=r->x+r->width;
        p2.y=r->y+r->height;
        //
        faceRoi.x=p1.x;
        faceRoi.y=p1.y;
        faceRoi.width=r->width;
        faceRoi.height=r->height;

        //cvRectangle(img,p1,p2,color,2);
        /////////////////////////////////////////////////////////

        ///////////////////////人眼矩形//////////////////////////
        //CvPoint p3,p4;
        p3.x=p1.x+r->width*0.22;
        p3.y=p1.y+r->height*0.32;
        p4.x=p1.x+r->width*0.78;
        p4.y=p1.y+r->height*0.50;

        eye.x=p3.x;
        eye.y=p3.y;
        eye.width=p4.x-p3.x;
        eye.height=p4.y-p3.y;

        LeyeRoi.x=p3.x;
        LeyeRoi.y=p3.y;
        LeyeRoi.width=(p4.x-p3.x)/2.3;
        LeyeRoi.height=p4.y-p3.y;

        ReyeRoi.width=(p4.x-p3.x)/2.3;
        ReyeRoi.height=p4.y-p3.y;
        ReyeRoi.x=p4.x-ReyeRoi.width;
        ReyeRoi.y=p3.y;
        //cvRectangle(img,p3,p4,color,2);
        /////////////////////////////////////////////////////////

        ///////////////////////鼻子矩形//////////////////////////
        //CvPoint p3,p4;
        p5.x=p1.x+r->width*0.20;
        p5.y=p1.y+r->height*0.40;
        p6.x=p1.x+r->width*0.75;
        p6.y=p1.y+r->height*0.80;

        noseRoi.x=p5.x;
        noseRoi.y=p5.y;
        noseRoi.width =p6.x-p5.x;
        noseRoi.height=p6.y-p5.y;
        /////////////////////////////////////////////////////////

        i++;
    }

    if(i!=1){return 1;}          //检测到多个人脸！

    ///////////////////////OTSU法阈值分割//////////////////////////
    //cout<<faceRoi.x<<" "<<faceRoi.width<<endl;
    //cout<<faceRoi.y<<" "<<faceRoi.height<<endl;

    //IplImage* faceimg = cvCreateImage(cvGetSize(img),img->depth,img->nChannels);
    Mat face = img ;

    //imshow("cao",face);

    Mat ycrcb;
    Mat channels[3];

    if(ycrcb.empty())
        ycrcb.create(face.rows,face.cols,CV_8UC3);

    cvtColor(face,ycrcb,CV_BGR2YCrCb);
    split(ycrcb,channels);

    if(otsu_cnt==5)
    {
        otsu = OTSU(channels[1]);
    }

    threshold(channels[1],channels[1],otsu,255,CV_THRESH_BINARY);

    Mat gray=channels[1];

    //IplImage* gray2 = cvCreateImage(cvGetSize(gray),gray->depth,gray->nChannels);
    //cvAdaptiveThreshold(gray, gray2, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 3, 5);
    //cvShowImage("gray2",gray2);
    //cvResetImageROI(gray);



    //cvThreshold(gray,gray,60,255,CV_THRESH_BINARY);

    Mat eyeROI = gray(eye);

    Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1) );
    morphologyEx(eyeROI,eyeROI,MORPH_OPEN,element);
    morphologyEx(eyeROI,eyeROI,MORPH_OPEN,element);
    morphologyEx(eyeROI,eyeROI,MORPH_OPEN,element);
    //morphologyEx(eyeROI,eyeROI,MORPH_OPEN,element);
    //morphologyEx(eyeROI,eyeROI,MORPH_CLOSE,element);
    //morphologyEx(eyeROI,eyeROI,MORPH_CLOSE,element);


    Mat noseROI = gray(noseRoi);
    //cvErode (gray,gray, NULL,2); //鼻孔腐蚀效果
    //imshow("o",eyeROI);
    //imshow("en",gray);
    //cvDilate(gray,gray, NULL,1);

    //imshow("en",gray);

    Mat pImg = gray;
    //////////////////////////左眼检测///  ///////////////////////
    Mat LeyeROI = pImg(LeyeRoi);
    //Mat LeyeImg = Mat::zeros(pImg.size(),CV_8UC1);
    //cvErode( LeyeImg,LeyeImg, NULL,1); //眼球腐蚀效果
    //cvDilate(LeyeImg,LeyeImg,NULL,2);
    //cvShowImage("L",LeyeImg);

    pxy = eyeShadowXY(LeyeROI); //灰度投影
    //if(pxy[3]==3)return 8;
    //if(pxy[3]==4)return 9;
    Leye.x=pxy[0]+LeyeRoi.x;
    Leye.y=pxy[1]+LeyeRoi.y;
    ////////////////////////////////////////////////////////////
    //////////////////////////右眼检测//////////////////////////
    Mat ReyeROI = pImg(ReyeRoi);
    //Mat ReyeImg = Mat::zeros(pImg.size(),CV_8UC1);
    //IplImage* ReyeImg = cvCreateImage(cvGetSize(pImg),8,1);
    //cvCopy(pImg,ReyeImg);
    //cvErode( ReyeImg,ReyeImg, NULL,1); //眼球腐蚀效果
    //cvDilate(ReyeImg,ReyeImg,NULL,2);
    //cvShowImage("R",ReyeImg);
    pxy = eyeShadowXY(ReyeROI); //灰度投影
    //if(pxy[3]==3)return 18;
    //if(pxy[3]==4)return 19;
    Reye.x=pxy[0]+ReyeRoi.x;
    Reye.y=pxy[1]+ReyeRoi.y;
    //cvResetImageROI(pImg);
    ////////////////////////////////////////////////////////////
    //////////////////////////鼻尖检测//////////////////////////
    //cvSetImageROI( img , noseRoi );

    int ni = 0;
    vector<Rect> noses;
    //IplImage* ngray = cvCreateImage(cvGetSize(img),8,1);
    //cvCvtColor( img, ngray, CV_BGR2GRAY );
    Mat ngray =  fgray(noseRoi);
    //cvtColor( img, ngray, CV_BGR2GRAY );
    equalizeHist( ngray, ngray );
    ncascade.detectMultiScale( ngray , noses,
                               1.4, 3,
                               CV_HAAR_FIND_BIGGEST_OBJECT
                               |CV_HAAR_DO_ROUGH_SEARCH
                               |CV_HAAR_SCALE_IMAGE
                               ,
                               Size(20, 20),Size(40,40) );

    //cvResetImageROI(img);

    for( vector<Rect>::const_iterator r = noses.begin(); r != noses.end(); r++)
    {
        nose.x = r->x + r->width/2+noseRoi.x;
        nose.y = r->y + r->height/2+noseRoi.y;
        ni++;
    }

    if(ni!=1){return 2;}

    ////////////////////////////////////////////////////////////

    //cvCircle(img,Leye,2,CV_RGB(255,255,0),2);
    //cvCircle(img,Reye,2,CV_RGB(255,255,0),2);
    //cvCircle(img,nose,2,CV_RGB(255,255,0),2);

    //cout<<Leye.y<<endl;
    //cvShowImage("T",pImg);
    ///////////////////////////////////////////////////////////////

    //cvRectangle( img,p5,p6,CV_RGB(0,0,0),2);
    //cvRectangle(gray,p5,p6,CV_RGB(0,0,0),2);

    //cvShowImage( "result", img );
    //cvShowImage("gray",gray);

    return 0;
}

int OTSU(Mat &img)
{
    float histogram[256]={0};
    for(int i=0;i<img.rows;i++)
    {
        const unsigned char* p=(const unsigned char*)img.ptr(i);
        for(int j=0;j<img.cols;j++)
        {
            histogram[p[j]]++;
        }
    }

    float avgValue = 0;
    int numPixel = img.cols*img.rows;
    for(int i=0;i<256;i++)
    {
        histogram[i] = histogram[i]/numPixel;
        avgValue += i*histogram[i];
    }

    int threshold = 0;
    float gmax=0;
    float wk=0,uk=0;
    for(int i=0;i<256;i++) {

        wk+=histogram[i];
        uk+=i*histogram[i];

        float ut=avgValue*wk-uk;
        float g=ut*ut/(wk*(1-wk));

        if(g > gmax)
        {
            gmax = g;
            threshold = i;
        }
    }
    //return 130;
    return threshold;
}

int* eyeShadowXY(Mat src)
{
    const int step = 5;
    const int halfstep=2;
    long sum=0;
    //imshow("hehe",src);

    //cout<<src.depth()<<endl;

    int xy[4];
    int xlast=0,maxsumx=0,sumx=0;
    int ylast=0,maxsumy=0,sumy=0;

    //Mat paintx=Mat::zeros(src.size(),CV_8UC1);
    //Mat painty=Mat::zeros(src.size(),CV_8UC1);
    //cvZero(paintx); //清空
    //cvZero(painty);
    int* v=new int[src.cols];  //开辟宽度大小的内存空间，数据类型是int,转换内存时需要*4，一个int占4个字节
    int* h=new int[src.rows];  //整型数组
    memset(v,0,src.cols*4);  //把所有位置都置0
    memset(h,0,src.rows*4); //将已经开辟内存空间v,h的字节数全部设置为0

    int x,y;
    unsigned char valGray;
    //CvScalar s; //数据容器

    /////////////////////统计X轴上灰度投影/////////////////////
    for(x=0;x<src.cols;x++)
    {
        for(y=0;y<src.rows;y++)
        {
            //s=cvGet2D(src,y,x); //返回指定的数组元素
            valGray = src.at<unsigned char>(y,x);
            if(valGray==0)
                v[x]++;
        }
        /////////////////////开始寻找波峰/////////////////////
        if(sum<10)xy[3]=3;
        if(sum>50)xy[3]=4;
        //cout<<sum<<endl;
        if(x)v[x]=(0.5*v[x]+0.5*v[x-1]);
        if(x<src.cols)
        {
            if(x<step){sumx+=v[x];}
            else if(x==step){xlast=v[x];sumx+=v[x];}
            else if(x>step){sumx-=xlast;sumx+=v[x];xlast=v[x];if(sumx>maxsumx){maxsumx=sumx,xy[0]=x-halfstep;}}
        }
        //////////////////////////////////////////////////////
    }
    ///////////////////////////////////////////////////////////
    /////////////////////绘制X轴上的统计图/////////////////////
    //for(x=0;x<src->width;x++)
    //{
    //    for(y=0;y<v[x];y++)
    //    {
    //		t.val[0]=255;
    //        cvSet2D(paintx,y,x,t);
    //    }
    //}
    ///////////////////////////////////////////////////////////

    for(y=0;y<src.rows;y++)
    {
        //valGray = src.ptr<unsigned char>(y);
        for(x=0;x<src.cols;x++)
        {
            valGray = src.at<unsigned char>(y,x);
            if(valGray==0)
                h[y]++;
        }
        if(y)h[y]=(0.5*h[y]+0.5*h[y-1]);
        if(y<src.rows)
        {
            if(y<step){sumy+=h[y];}
            else if(y==step){ylast=h[y];sumy+=h[y];}
            else if(y>step){sumy-=ylast;sumy+=h[y];ylast=h[y];if(sumy>maxsumy){maxsumy=sumy,xy[1]=y-halfstep;}}
        }
    }

    //for(y=0;y<src->height;y++)
    //{
    //	for(x=0;x<h[y];x++)
    //    {
    //		t.val[0]=255;
    //		cvSet2D(painty,y,x,t);
    //	}
    //}

    //cvFlip(paintx,paintx,0);

    //cvShowImage("二值图像",src);
    //cvShowImage("垂直积分投影",paintx);
    //cvShowImage("水平积分投影",painty);

    return xy;
}
int* noseShadowXY(IplImage* src)
{
    const int step = 5;
    const int halfstep=2;

    int extre=0;                  //检测到投影图波谷
    int extre_cnt=0;
    int extre_cnt_th=3;
    const int Th=2;

    int xy[2];
    int Lx=0,Rx=0;
    int ylast=0,maxsumy=0,sumy=0;

    IplImage* paintx=cvCreateImage( cvGetSize(src),IPL_DEPTH_8U, 1 );
    IplImage* painty=cvCreateImage( cvGetSize(src),IPL_DEPTH_8U, 1 );
    cvZero(paintx); //清空
    cvZero(painty);
    int* v=new int[src->width];  //开辟宽度大小的内存空间，数据类型是int,转换内存时需要*4，一个int占4个字节
    int* h=new int[src->height];  //整型数组
    memset(v,0,src->width*4);  //把所有位置都置0
    memset(h,0,src->height*4); //将已经开辟内存空间v,h的字节数全部设置为0

    int x,y;
    CvScalar s; //数据容器

    /////////////////////统计X轴上灰度投影/////////////////////
    for(x=0;x<src->width;x++)
    {
        for(y=0;y<src->height;y++)
        {

            s=cvGet2D(src,y,x); //返回指定的数组元素
            if(s.val[0]==0)
                v[x]++;
        }
        /////////////////////开始寻找波峰/////////////////////
        if(x)v[x]=v[x]*v[x]/5;
        if(extre==0)
        {
            if(v[x]>=Th)extre_cnt+=1;
            //else extre_cnt-=1;
            if(extre_cnt>=extre_cnt_th)
            {
                extre_cnt=0;
                extre=1;
            }
        }
        else if(extre==1)
        {
            if(v[x]<Th)extre_cnt+=1;
            if(extre_cnt>=extre_cnt_th)
            {
                extre_cnt=0;
                extre=2;
                Lx=x;
            }
        }
        else if(extre==2)
        {
            if(v[x]>=Th)extre_cnt+=1;
            if(extre_cnt>=extre_cnt_th)
            {
                extre=3;
                Rx=x;
            }
        }

        xy[0]=(Lx+Rx)/2;

        //////////////////////////////////////////////////////
    }
    ///////////////////////////////////////////////////////////
    /////////////////////绘制X轴上的统计图/////////////////////
    //for(x=0;x<src->width;x++)
    //{
    //    for(y=0;y<v[x];y++)
    //    {
    //		t.val[0]=255;
    //        cvSet2D(paintx,y,x,t);
    //    }
    //}
    ///////////////////////////////////////////////////////////


    for(y=0;y<src->height;y++)
    {
        for(x=0;x<src->width;x++)
        {
            s=cvGet2D(src,y,x);
            if(s.val[0]==0)
                h[y]++;
        }
        if(y)h[y]=(h[y]+h[y-1])/2;
        if(y<src->height)
        {
            if(y<step){sumy+=h[y];}
            else if(y==step){ylast=h[y];sumy+=h[y];}
            else if(y>step){sumy-=ylast;sumy+=h[y];ylast=h[y];if(sumy>maxsumy){maxsumy=sumy,xy[1]=y-halfstep;}}
        }
    }

    //for(y=0;y<src->height;y++)
    //{
    //	for(x=0;x<h[y];x++)
    //    {
    //		t.val[0]=255;
    //		cvSet2D(painty,y,x,t);
    //	}
    //}

    //cvFlip(paintx,paintx,0);

    //cvShowImage("二值图像",src);
    //cvShowImage("垂直积分投影",paintx);
    //cvShowImage("水平积分投影",painty);

    return xy;
}
int standard_y=0;
Point last_nos;
int nose_y=0;
struct op{
    int op_num;
    int op_x[5];
    int op_y[5];
}op;
int op_temp[2];
int out_op[2];

int last_face_x;

void GetDirection(CvPoint Eye1,CvPoint Eye2,CvPoint Nose)
{
    int face_x=0;
    if( abs(Nose.y-last_nos.y) > THDIRECY )
    {
        if( Nose.y > last_nos.y )
        {

            if( abs(Nose.y-last_nos.y) > 2*THDIRECY )     {nose_y+=2;}
            else if( abs(Nose.y-last_nos.y) > THDIRECY )  {nose_y+=1;}
        }
        if( Nose.y < last_nos.y )
        {
            if( abs(Nose.y-last_nos.y) > 2*THDIRECY )     {nose_y-=2;}
            else if( abs(Nose.y-last_nos.y) > THDIRECY )  {nose_y-=1;}
        }
    }
    if(nose_y>2)nose_y=2;
    if(nose_y<-2)nose_y=-2;
    last_nos = Nose;

    /*
    switch(nose_y)
    {
    case -2:{op_temp[1]=0;break;}
    case -1:{op_temp[1]=1;break;}
    case  0:{op_temp[1]=2;break;}
    case  1:{op_temp[1]=3;break;}
    case  2:{op_temp[1]=4;break;}
    }
    */
    CvPoint Pedal ;
    int x1,x2,x3;
    int y1,y2,y3;
    x1=Eye1.x; y1=Eye1.y;
    x2=Eye2.x; y2=Eye2.y;
    x3=    Nose.x; y3=    Nose.y;

    //Pedal.y = (int)( ( (x2-x1)*(y2-y1)*x3 + (y2-y1)*(y2-y1)*y3 + (x2*y1-x1*y2)*(x2-x1) ) / ( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) ) );
    //Pedal.x = (int)( x3 - ( ( (y1-y2)*(Pedal.y-y3) ) / (x1-x2) ) );

    Pedal.x = Face.x;
    //Pedal.y = last_nose.y;

    //int   eye_dx  = abs(Eye1.x-Eye2.x);
    //int   eye_dy  = abs(Eye1.y-Eye2.y);
    //float mid_y   = (Eye1.y+Eye2.y)/2;
    //float mid_x   = (Eye1.x+Eye2.x)/2;
    float mid_y   = nose.y;
    float mid_x   = nose.x;
    //int   eye_L   = (eye_dx*eye_dx) + (eye_dy*eye_dy);


    if(mid_x > (Pedal.x+THDIRECX))
    {
        if      (mid_x > (Pedal.x+9)){face_x=-4;}//cout<<"→→→"<<endl;
        else if (mid_x > (Pedal.x+7)){face_x=-3;}
        else if (mid_x > (Pedal.x+5)){face_x=-2;}//cout<<"→→"<<endl;
        else                         {face_x=-1;}//cout<<"→"<<endl;}
    }
    else if(mid_x < (Pedal.x-THDIRECX))
    {
        if      (mid_x < (Pedal.x-9)){face_x=4;}//cout<<"←←←"<<endl;
        else if (mid_x < (Pedal.x-7)){face_x=3;}
        else if (mid_x < (Pedal.x-5)){face_x=2;}//cout<<"←←"<<endl;
        else                         {face_x=1;}//cout<<"←"<<endl;
    }
    else
    {face_x=0;}

    /*
    if(op.op_num<5)
    {
        op.op_x[0]=op.op_x[1];op.op_y[0]=op.op_y[1];
        op.op_x[1]=op.op_x[2];op.op_y[1]=op.op_y[2];
        op.op_x[2]=op.op_x[3];op.op_y[2]=op.op_y[3];
        op.op_x[3]=op.op_x[4];op.op_y[3]=op.op_y[4];
        op.op_x[4]=op_temp[0];op.op_y[4]=op_temp[1];
    }

    if(op.op_x[4]==op.op_x[3])out_op[0]=op.op_x[4];
    if(op.op_y[4]==op.op_y[3])out_op[1]=op.op_y[4];
    */
    /////////////////////////////////
    //  						   //
    //   x:1:->->->    y:0:↑↑	   //
    //     2:->->        1:↑	   //
    //     3:->          2: ---	   //
    //     4:<-<-<-      3:↓	   //
    //     5:<-<-        4:↓↓	   //
    //     6:<-					   //
    //     7:------				   //
    //							   //
    /////////////////////////////////

    //cout<<out_op[0]<<endl;
    //cout<<out_op[1]<<endl;


    if(!start)
    {
        if(last_face_x==face_x)
        {
            last_face_x=face_x;
        }
        else
        {
            int temp=last_face_x;
            last_face_x=face_x;
            face_x=last_face_x;
        }
    }

    img = Mat::zeros(Size(640,480),CV_8UC3);
    if(face_x>=0 && nose_y>=0)rectangle(img,Point(320+80*(face_x-1),240+80*nose_y),Point(320+80*face_x,240+80*(nose_y-1)),Scalar(255,0,255),-1);
    if(face_x<=0 && nose_y>=0)rectangle(img,Point(320+80*face_x,240+80*nose_y),Point(320+80*(face_x+1),240+80*(nose_y-1)),Scalar(255,0,255),-1);

    if(face_x>=0 && nose_y<=0)rectangle(img,Point(320+80*(face_x-1),240+80*nose_y),Point(320+80*face_x,240+80*(nose_y+1)),Scalar(255,0,255),-1);
    if(face_x<=0 && nose_y<=0)rectangle(img,Point(320+80*face_x,240+80*nose_y),Point(320+80*(face_x+1),240+80*(nose_y+1)),Scalar(255,0,255),-1);
    /*
    Point eye1,eye2,nose,nose1,nose2;
    int eyedx = abs(x2-x1)/2;
    float angle = atan((float)(y2-y1)/(x2-x1))*180/3.14;
    eye1.x=80-eyedx;
    eye2.x=80+eyedx;
    nose.x=eye1.x+(x3-x1)/2;

    eye1.y=Eye1.y>>2;
    eye2.y=Eye2.y>>2;
    nose.y=(eye1.y+eye2.y)/2+eyedx*(Nose.y/(Eye1.y+Eye2.y)/2);

    nose1=nose2=nose;
    nose1.x -= 2;
    nose2.x += 2;

    Rect face3d;
    face3d.x=480;
    face3d.y=280;
    face3d.width=160;
    face3d.height=200;

    Mat characters = img(face3d);

    characters.setTo(0);
    circle(characters,eye1,4,Scalar(255),-1);
    ellipse(characters,eye1,Size(9,4),angle,0,360,Scalar(255),1);
    circle(characters,eye2,4,Scalar(255),-1);
    ellipse(characters,eye2,Size(9,4),angle,0,360,Scalar(255),1);
    circle(characters,nose1,2,Scalar(255),-1);
    circle(characters,nose2,2,Scalar(255),-1);
    */
}



