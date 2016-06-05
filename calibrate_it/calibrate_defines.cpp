#include "calibrate_defines.h"

#include <QDesktopWidget>
#include <QRect>
#include <QFont>
#include <Windows.h>

#include <opencv2/highgui/highgui.hpp>

const qint16 smallFontSize   = 8;
const qint16 originFontSize  = 10;
const qint16 boldFontSize    = 11;

////////////////////////////////////////////////////////////////
QPoint Globals::desktop;
QFont* Globals::originFont = NULL;
QFont* Globals::smallFont  = NULL;
QFont* Globals::boldFont   = NULL;

void Globals::Init()
{
    QDesktopWidget desk;
    QRect rcScreen( desk.screenGeometry() );
    desktop.rx() = rcScreen.width()+1;
    desktop.ry() = rcScreen.height()+1;

    smallFont = new QFont();
    smallFont->setPixelSize(smallFontSize);

    originFont = new QFont();
    originFont->setPixelSize(originFontSize);

    boldFont = new QFont();
    boldFont->setPixelSize(boldFontSize);
    boldFont->setBold(true);
}

static HWND testWindow = 0;
static int nshow = 0;

extern void showTestImage(const CvMat& mat)
{
    if( !testWindow ) {
        cvNamedWindow("interop");
        testWindow = (HWND)cvGetWindowHandle("interop");
        ::SetWindowPos(testWindow,HWND_TOPMOST,0,0,0,0,SWP_SHOWWINDOW|SWP_NOSIZE);
    }

    if( nshow++ ) {
        if(nshow > 15)
            nshow = 0;
        return;
    }

    cvShowImage("interop",&mat);
    UpdateWindow(testWindow);
}

extern void showTestImage(const cv::Mat& bits) 
{
    showTestImage(CvMat(bits));
}
