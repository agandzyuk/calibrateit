#include "calibrate_defines.h"
#include "movie2stereo_matcher.h"

#include <QtCore>
#include <QOpenGLFunctions>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

using namespace cv;

M2StereoMatcher::M2StereoMatcher(QObject* parent, QOpenGLFunctions* glfx)
    : QThread(parent),
    container_(new T2FrameContainer(glfx, 5)),
    shutdown_(false),
    glfx_(glfx)
{}

M2StereoMatcher::~M2StereoMatcher()
{
    if( !shutdown_ )
        stop();
}

void M2StereoMatcher::start()
{
    QThread::start();
}

void M2StereoMatcher::stop()
{
    {
        QWriteLocker lock(&rwLock_);
        shutdown_ = true;
    }
    wait();
}


bool M2StereoMatcher::bind(const Mat& frame)
{
    // back buffer contins the pre-match image
    if( back_.isNull() )
        back_.reset(new Mat(frame.rows, frame.cols, frame.type()));
    frame.copyTo(*back_.data());

    QReadLocker g(&rwLock_);
    if( disparity_.isNull() || disparity_->empty() )
        container_->copyFrom(frame); // skip matching and fetch current frame
    else
        container_->copyFrom(*disparity_.data()); // matched

    return true;
}

void M2StereoMatcher::run()
{
    bool shutdownFlag = false;
    while( !shutdownFlag )
    {
        {
            QReadLocker g(&rwLock_);
            if( true == (shutdownFlag = shutdown_) )
                break; // shutdowning occured during the thread was locked: stop execution
        }

        if( back_.isNull() || back_->empty() ) 
        {
            QThread::msleep(50); // idle
        }
        else
        {
            calculate(); // match
        }
    }
}

void M2StereoMatcher::calculate()
{
    cv::Mat currGrey(back_->rows, back_->cols, CV_8UC1);
    // convert or copy
    int nChannels = back_->channels();
    if( nChannels > 1) {
        cv::cvtColor(*back_.data(), currGrey, CV_RGBA2GRAY);
        //cvThreshold(&CvMat(currGrey), &CvMat(currGrey), 150, 250, CV_THRESH_BINARY);
    }

    // no match when no previous frame or back buffering not occured
    if( front_.isNull() )
        front_.reset(new Mat(currGrey.rows, currGrey.cols, currGrey.type()));

    if( front_->empty() ) {
        currGrey.copyTo(*front_.data());
        return;
    }

    CvStereoBMState* sbs = cvCreateStereoBMState(CV_STEREO_BM_BASIC);
    //Prefilters
    sbs->preFilterType = 0;
    sbs->preFilterSize = 5; // ~5×5..21×21
    sbs->preFilterCap = 13; // < ~31
    //
    sbs->SADWindowSize = 7; // 5×5..21×21
    sbs->minDisparity = -16;
    sbs->numberOfDisparities = 16;

    //Postfilters
    sbs->textureThreshold = 13; // ignore areas without texture
    sbs->uniquenessRatio = 10; // filter output pixes
    sbs->speckleWindowSize = 0;
    sbs->speckleRange = 0;

    const char* excs;
    try {
        cv::Mat disparity16(front_->rows, front_->cols, CV_16SC1); 
        cvFindStereoCorrespondenceBM( &CvMat(*front_.data()), 
                                      &CvMat(currGrey), 
                                      &CvMat(disparity16), sbs);
        if( disparity_.isNull() )
            disparity_.reset(new Mat(back_->rows, back_->cols, back_->type()));
        disparity16.convertTo(disparity16, CV_8UC1);

        // modify objects safely
        QWriteLocker wlock(&rwLock_);
        cvtColor(*front_.data(),*disparity_.data(),CV_GRAY2RGBA);
        cvMerge(&CvMat(disparity16),0,0,0,&CvMat(*disparity_.data()));
    }
    catch(const cv::Exception& ex) {
        excs = ex.msg.c_str();
        qDebug() << excs;
        return;
    }
    currGrey.copyTo(*front_.data());
}
