#ifndef HEADCTRL_H
#define HEADCTRL_H
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <QDialog>
#include <QHBoxLayout>
#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QTimer>
#include <QLabel>
#include <QGridLayout>
#include <QPainter>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include "facedet.h"


using namespace cv;

class Headctrl : public QWidget
{
    Q_OBJECT
public:
    explicit Headctrl(QWidget *parent = 0);


signals:

private slots:
    void headctrlstart();
    void headctrlend();

private:
    QPushButton *ctl_on_btn;
    QPushButton *ctl_off_btn;
    QGridLayout *ctl_layout;
    QVBoxLayout *ctl_sublayout;

    IplImage *handleimg;
    QTimer *timer;
};

void getframe();

#endif // HEADCTRL_H
