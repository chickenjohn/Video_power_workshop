#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <QWidget>
#include <QTabWidget>
#include <QTimer>
#include <QPixmap>
#include <QLabel>
#include "headctrl.h"
#include "drawer.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <QTimer>
#include "mousectl.h"

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = 0);
    ~MainDialog();

public slots:
    void startcapture();
    void pagechange(int index);
    void changeclick();

signals:
   void draw_end_sig(void);

private:
    QTabWidget *tab;
    QLabel * state_label;
    Headctrl *headctrl;
    Drawer *drawer;
};



#endif // MAINDIALOG_H
