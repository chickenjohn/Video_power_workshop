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

#include "maindialog.h"

int INIT_MOUSE=1;

CvCapture *capture;
IplImage *frame;
QTimer *mouse_timer;
QTimer  *timer;
int state=4;
int pen_not_found_counter=0;
int finger_not_found_counter=0;
CascadeClassifier cascade,ncascade;//创建级联分类器对象
//String cascadeName = "./lbpcascade_frontalface.xml";
String cascadeName = "./haarcascade_frontalface_alt2.xml";
String NosecascadeName = "./haarcascade_mcs_nose.xml";

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
{
    mouse_timer = new QTimer;
    mouse_timer->start(6000);
    //layout
    this->resize(640,480);
    setWindowTitle(tr("Video Power Workshop--fantastic video based workshop"));

    headctrl = new Headctrl;
    drawer = new Drawer;
    tab = new QTabWidget;
    state_label = new QLabel;
    tab ->addTab(headctrl, tr("Head Controller"));
    tab -> addTab(drawer, tr("Power Drawer"));
    state_label->setText(tr("None"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tab);
    mainLayout->addWidget(state_label);
    setLayout(mainLayout);

    cascade.load( cascadeName );
    ncascade.load( NosecascadeName );

    timer = new QTimer;
    capture = cvCaptureFromCAM(0);
    if(capture==NULL)
    {
        qDebug()<<"error!";
    }

    connect(timer,SIGNAL(timeout()),this,SLOT(startcapture()));
    connect(tab,SIGNAL(currentChanged(int)),this,SLOT(pagechange(int)));
    connect(this,SIGNAL(draw_end_sig()),drawer,SLOT(enddraw()));
    connect(mouse_timer,SIGNAL(timeout()),this, SLOT(changeclick()));

    timer->start(110);
}

void MainDialog::startcapture()
{
    int finger_init_success=-1;
    frame = cvQueryFrame(capture);
    extern bool object_found;
    extern bool isWritting;
    extern QLabel * number_rec;
    switch(state)
    {
    case 0:
        state_label->setText(tr("Mouse"));
        if(INIT_MOUSE)
        {
            finger_init_success = initdirect();
            if(finger_init_success<0)
            {
                INIT_MOUSE=1;
            }
            else
                INIT_MOUSE=0;
        }
        else
            mouse_ctrl();
        break;
    case 1:
        state_label->setText(tr("Head"));
        getframe();
        break;
    case 2:
        state_label->setText(tr("Drawer"));
        startdraw();
        system("clear");
        printf("objf=%d\n",object_found);
        if(!object_found)
            pen_not_found_counter++;
        else
            pen_not_found_counter=0;
        if(!isWritting)
            number_rec->setText(tr("Not Writting"));
        else
            number_rec->setText(tr("Writting"));
        //mouse_ctrl();
        break;
    case 3:
        break;
    default:
        state_label->setText(tr("None"));
        break;
    }
    if(pen_not_found_counter>25)
    {
        pen_not_found_counter=0;
        state=0;
        emit draw_end_sig();
    }
    if(finger_not_found_counter>20)
    {
        finger_not_found_counter=0;
        state=2;
    }
}

void MainDialog::pagechange(int index)
{
    pen_not_found_counter=0;
    finger_not_found_counter=0;
    if(index==1)
    {
        timer->start(60);
        state = 2;
    }
    else if(index == 0)
    {
        timer->start(110);
        state = 4;
    }
}

void MainDialog::changeclick()
{
    extern bool mouse_btn;
    mouse_btn=true;
}

MainDialog::~MainDialog()
{

}
