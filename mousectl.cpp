#include "mousectl.h"

using namespace std;
using namespace cv;

bool ctrl=false;
bool eraser=false;
bool mouse_btn=false;
bool INIT=false;
bool cleaner= false;
Point mouse_pos;


int Khand=0;
Point last_finger;
Point next_finger[2];
Mat input_image;
Mat output_mask;
Mat output_image;
static Mat mask;
Mat skinCrCbHist = Mat::zeros(Size(256, 256), CV_8UC1);
Point last_eraser=Point{0,0};

Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1) );

static int oldx=0,oldy=0;

static struct finger
{
    int num;
    Point f[10];
}finger[15];

void mouse_ctrl()
{
    int rel_x,rel_y;
    int fd_mouse = -1;

    getdirect(&rel_x,&rel_y);
    //fd_mouse = open("/dev/input/event17",O_RDWR);
    //for zybo
    fd_mouse = open("/dev/input/event0",O_RDWR);
    if(fd_mouse<=0)
    {
        printf("error!\n");
    }
    cout<<"mouse_btn = "<<mouse_btn<<endl;
    if(mouse_btn)
    {
        simulate_key(fd_mouse, BTN_LEFT);
        mouse_btn = false;
    }
    simulate_mouse(fd_mouse,rel_x,rel_y);
    close(fd_mouse);
}

void getdirect(int *rel_x, int *rel_y)
{
    int x,y;
    int if_success = -1;
    extern int finger_not_found_counter;

    if_success = detfinger(&x,&y);
    if(if_success<0)
    {
        *rel_x = 0;
        *rel_y = 0;
        finger_not_found_counter++;
    }
    else
    {
        finger_not_found_counter=0;
        x = 0.3*x+oldx*0.7;
        y = 0.3*y+oldy*0.7;
        *rel_x = (int)((x - oldx)*1.4);
        *rel_y = (int)((y - oldy)*1.4);

        if(cleaner)
        {
            extern IplImage *warpCanvas;
            extern QLabel  *paintarea;
            extern CvMat * warp_mat;
            CvPoint real_point, real_old_point;
            real_old_point = transformPoint(cvPoint(oldx,oldy),warp_mat);
            real_point = transformPoint(cvPoint(x,y),warp_mat);
            real_old_point.x*=2;
            real_old_point.y*=2;
            real_point.x*=2;
            real_point.y*=2;
            cvLine(warpCanvas,real_old_point,real_point,CV_RGB(255,255,255),20,8,0);
            cvCvtColor(warpCanvas , warpCanvas, CV_BGR2RGB);
            QImage image((const uchar*)warpCanvas->imageData,warpCanvas->width,warpCanvas->height,QImage::Format_RGB888);
            paintarea->setPixmap(QPixmap::fromImage(image));
        }
        oldx = x;
        oldy = y;
    }
}



int initdirect()
{
    extern int finger_not_found_counter;
    int if_success=0;
    if_success = detfinger(&oldx,&oldy);
    if(if_success<0)
        finger_not_found_counter++;
    return if_success;
}

int detfinger(int *x, int *y)
{
    extern IplImage * frame;
    Mat input_image(frame,0);
    ellipse(skinCrCbHist, Point(113, 155.6), Size(23.4, 15.2), 43.0, 0.0, 360.0, Scalar(255, 255, 255), -1);

    resize(input_image,input_image,Size(320,240));
    Mat ycrcb_image;
    output_mask = Mat::zeros(input_image.size(), CV_8UC1);
    cvtColor(input_image, ycrcb_image, CV_BGR2YCrCb); //首先转换成到YCrCb空间

    for(int i = 0; i < input_image.rows; i++) //利用椭圆皮肤模型进行皮肤检测
    {
        uchar* p = (uchar*)output_mask.ptr<uchar>(i);
        Vec3b* ycrcb = (Vec3b*)ycrcb_image.ptr<Vec3b>(i);
        for(int j = 0; j < input_image.cols; j++)
        {
            if(skinCrCbHist.at<uchar>(ycrcb[j][1], ycrcb[j][2]) > 0)
                p[j] = 255;
        }
    }

    morphologyEx(output_mask,output_mask,MORPH_CLOSE,element);
    morphologyEx(output_mask,output_mask,MORPH_CLOSE,element);
    vector< vector<Point> > contours;   // 轮廓
    vector< vector<Point> > filterContours; // 筛选后的轮
    vector< Vec4i > hierarchy;    // 轮廓的结构信息

    contours.clear();
    hierarchy.clear();
    filterContours.clear();
    findContours(output_mask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    // 去除伪轮廓
    for (size_t i = 0; i < contours.size(); i++)
    {
        Khand=fabs(contourArea(Mat(contours[i])));
        if (Khand >= 2000/*&&fabs(arcLength(Mat(contourrs[i]),true))<2000*/)  //判断手进入区域的阈值
        {
            approxPolyDP(contours[i],contours[i],15,1);
            filterContours.push_back(contours[i]);
            break;
        }
    }
    if(Khand < 2000)return -1;
    Khand*=0.2;

    //output_mask.setTo(0);
    //drawContours(output_mask, filterContours, -1, Scalar(255), 1); //8, hierarchy);
    int m10=0;
    int m00=0;
    int m01=0;

    Moments moment;
    moment = moments(output_mask);
    m10 = moment.m10;
    m01 = moment.m01;
    m00 = moment.m00;

    Point G;

    if(m00!=0)
    {
        G.x = m10/m00;
        G.y = m01/m00;
    }
    else return -1;

    circle(output_mask,G,2,Scalar(255),5,8,0);

    for(int i1=0;i1<14;i1++)
    {
        finger[i1].num=finger[i1+1].num;
        for(int j1=0;j1<finger[i1+1].num;j1++)
        {
            finger[i1].f[j1]=finger[i1+1].f[j1];
        }
    }

    bool first=true;
    int i=0;
    int xp=0;

    Point points[100];

    for( vector<Point>::const_iterator r = filterContours[0].begin(); r != filterContours[0].end(); r++)
    {
        points[xp]=Point(r->x,r->y);
        xp++;
        //circle(output_mask,points[])
    }

    if(xp==0)return -1;

    for( int xpk=0;xpk<xp;xpk++)
    {
        Point K = points[xpk];
        if(K.y>G.y)continue;
        if(first)
        {
            if( distance2(K,G)>Khand )
            {
                first=false;
                if(xpk==0)
                    next_finger[0] = points[xp-1];
                else
                    next_finger[0] = points[xpk-1];
                last_finger=K;
                next_finger[1] = points[xpk+1];
            }
        }
        else
        {
            if( distance2(K,G)>Khand)
            {
                if( distance2(K,last_finger)>(Khand/4) )
                {
                    next_finger[0] = points[xpk-1];
                    last_finger=K;
                    if(xpk != xp-1)
                        next_finger[1] = points[xpk+1];
                    else next_finger[1]= points[0];
                    i++;
                }
                else
                {
                    if(distance2(K,G)>distance2(last_finger,G))
                    {
                        next_finger[0] = points[xpk-1];
                        last_finger = K;
                        if(xpk != xp-1)
                            next_finger[1] = points[xpk+1];
                        else next_finger[1]= points[0];
                    }
                }

                //char t[256];
                //string s;
                //sprintf(t, "%d", i);
                //s = t;
                //putText(output_mask,s,last_finger,0,1,Scalar(255));
            }
        }

        if(!Kcompare())continue;

        circle(output_mask,last_finger,2,Scalar(255),2,8,0);
        line(output_mask,last_finger,G,Scalar(255),2);

        finger[14].num=i+1;
        finger[14].f[i]=last_finger;

    }

    int gesture=0;

    extern CvPoint2D32f corners[4];
    gesture=getgesture();
    // imshow("out",output_mask);
    printf("finger[14].num=%d",finger[14].num);
    if(finger[14].num==1)
    {
        if(gesture==4||gesture==5)
        {

            cleaner=true;
        }
        *x = last_finger.x;
        *y = last_finger.y;
        return 0;
    }
    else if(finger[14].num==2)
    {
        CvPoint mouse_pos_real;
        mouse_pos_real.x = 200;
        mouse_pos_real.y = 150;
        switch(gesture)
        {
        case 1: zoomIn(mouse_pos_real);break;
        case 2: zoomOut(mouse_pos_real);break;
        default: break;
        }
        return 0;

    }
    else
        return -1;
}

int distance2(Point a,Point b)
{
    int dx=abs(a.x-b.x);
    int dy=abs(a.y-b.y);
    return dx*dx+dy*dy;
}

bool Kcompare()
{
    int k1 =
            (last_finger.x-next_finger[0].x)*(last_finger.x-next_finger[0].x)
            +(last_finger.y-next_finger[0].y)*(last_finger.y-next_finger[0].y);
    int k2 =
            (last_finger.x-next_finger[1].x)*(last_finger.x-next_finger[1].x)
            +(last_finger.y-next_finger[1].y)*(last_finger.y-next_finger[1].y);


    int k3 =
            (next_finger[0].x-next_finger[1].x)*(next_finger[0].x-next_finger[1].x)
            +(next_finger[0].y-next_finger[1].y)*(next_finger[0].y-next_finger[1].y);

    if((k3*4) < k1+k2)return true;
    else return false;
}


int getgesture()
{
    int gesturek=0;
    //if(Kcompare())
    //////////////////////得到手指特点1个///////////////////////
    if(finger[14].num==1)
    {
        int fnum=0;
        mouse_pos.x = mouse_pos.x*0.5 + (finger[4].f[0].x)*0.5 ;
        mouse_pos.y = mouse_pos.y*0.5 + (finger[4].f[0].y)*0.5 ;

        for(int j=0;j<14;j++)
        {
            if(finger[j].num == 1 )
                fnum++;
        }

        if(fnum>=10)
        {
            if(
                    distance2(finger[0].f[0],finger[1].f[0])<5
                    &&  distance2(finger[1].f[0],finger[2].f[0])<5
                    &&  distance2(finger[2].f[0],finger[3].f[0])<5
                    &&  distance2(finger[3].f[0],finger[4].f[0])<5
                    &&  distance2(finger[4].f[0],finger[5].f[0])<5
                    &&  distance2(finger[5].f[0],finger[6].f[0])<5
                    &&  distance2(finger[6].f[0],finger[7].f[0])<5
                    &&  distance2(finger[7].f[0],finger[8].f[0])<5
                    &&  distance2(finger[8].f[0],finger[9].f[0])<5
                    &&  distance2(finger[9].f[0],finger[10].f[0])<5
                    &&  distance2(finger[10].f[0],finger[11].f[0])<5
                    &&  distance2(finger[11].f[0],finger[12].f[0])<5
                    &&  distance2(finger[12].f[0],finger[13].f[0])<5
                    &&  distance2(finger[13].f[0],finger[14].f[0])<5
                    )
                //cout<<"heheh"<<endl;
            {
                cout<<"nowx =  "<<finger[14].f[0].x<<" nowy = \n"<<finger[14].f[0].y<<endl;
                cout<<"lastx =  "<<last_eraser.x<<" lasty = \n"<<last_eraser.y<<endl;
                if(distance2(finger[14].f[0],last_eraser)>1600)
                {
                    eraser=!eraser;
                    last_eraser=finger[14].f[0];
                    gesturek= (eraser)? 4:5;
                }
            }
        }

        /*
        if(finger[3].num==2 && finger[2].num==2 && finger[1].num==2)
        {
            if( distance2( Point((finger[3].f[0].x+finger[3].f[1].x)/2,(finger[3].f[0].y+finger[3].f[1].y)/2) , finger[4].f[0] ) < 3600 )
            {
                //system("cls");
                mouse_btn=true;
                gesturek=3;
                ctrl=true;
                //cout<<"Get"<<endl;
            }
        }
        */

        //if(mouse_btn==true) circle(output_mask,finger[4].f[0],5,Scalar(255),5);
    }
    ////////////////////////////////////////////////////////////

    //////////////////////得到手指特点2个///////////////////////
    if(finger[14].num==2)
    {
        ctrl=false;
        eraser=false;


        mouse_pos.x = mouse_pos.x*0.5 + (finger[14].f[0].x+finger[14].f[1].x)/2*0.5 ;
        mouse_pos.y = mouse_pos.y*0.5 + (finger[14].f[0].y+finger[14].f[1].y)/2*0.5 ;

        if(finger[13].num==2)
        {
            int Lnow  = sqrt((float)distance2(finger[14].f[0],finger[14].f[1]));
            int Llast = sqrt((float)distance2(finger[13].f[0],finger[13].f[1]));

            if(abs(Lnow-Llast)>5)
            {
                if(Lnow>Llast) {system("clear");cout<<"←  →"<<endl;gesturek=1;}
                else           {system("clear");cout<<"→  ←"<<endl;gesturek=2;}
            }
            else system("clear");cout<<"|   |"<<endl;
        }

        if(finger[13].num==1 && finger[12].num==1)
        {
            if( distance2( Point((finger[14].f[0].x+finger[14].f[1].x)/2,(finger[14].f[0].y+finger[14].f[1].y)/2) , finger[13].f[0] ) < 3600 )
            {
                //system("cls");
                //cout<<"cancel"<<endl;
            }
        }

        //else {system("cls");}
    }
    ////////////////////////////////////////////////////////////

    //if(eraser)circle(output_mask,last_finger,10,Scalar(255),3);

    cout<<"eraser ="<<eraser<<"   ";
    //cout<<"Ctrl   ="<<eraser<<"   ";
    //cout<<"mouse_btn  ="<<mouse_btn<<"   ";

    //waitKey(10);

    //system("cls");

    if(finger[14].num!=2 && finger[14].num!=1) mouse_pos = Point(0,0);
    circle(output_mask,mouse_pos,5,Scalar(255),3);

    cout<<"k="<<gesturek<<endl;

    return gesturek;
    /////////////////////////////
    //    1:←  →		       //
    //	  2:→  ←		       //
    //    3:			       //
    //    4:eraser_down	}	   //
    //    5:eraser_up	}clk   //
    /////////////////////////////
}

void simulate_mouse(int fd, int rel_x, int rel_y)
{
    struct input_event event;
    gettimeofday(&event.time, 0);
    //x轴坐标的相对位移
    event.type = EV_REL;
    event.value = rel_x;
    event.code = REL_X;
    write(fd, &event, sizeof(event));
    //y轴坐标的相对位移
    event.type = EV_REL;
    event.value = rel_y;
    event.code = REL_Y;
    write(fd, &event, sizeof(event));                        //同步
    event.type = EV_SYN;
    event.value = 0;
    event.code = SYN_REPORT;
    write(fd, &event, sizeof(event));
}

void simulate_key(int fd, int kval)
{
    struct input_event event;
    gettimeofday(&event.time, 0);
    //按下kval键
    event.type = EV_KEY;
    event.value = 1;
    event.code = kval;
    write(fd, &event, sizeof(event));
    //同步，也就是把它报告给系统
    event.type = EV_SYN;
    event.value = 0;
    event.code = SYN_REPORT;
    write(fd, &event, sizeof(event));
    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, 0);
    //松开kval键
    event.type = EV_KEY;
    event.value = 0;
    event.code = kval;
    write(fd, &event, sizeof(event));
    //同步，也就是把它报告给系统
    event.type = EV_SYN;
    event.value = 0;
    event.code = SYN_REPORT;
    write(fd, &event, sizeof(event));
}
