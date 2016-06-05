#include "calibrate_defines.h"
#include "camera_stream_reader.h"
#include "texture2frame_container.h"
#include "calibrate_it.h"

#include <QtCore>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>


using namespace cv;

CameraStreamReader::CameraStreamReader(QObject* parent)
    : QThread(parent),
    shutdown_(false),
    histoAccuracy_(0,0),
    histoRGB_(eNone)
{
    start();
}

CameraStreamReader::~CameraStreamReader()
{
    if( !shutdown_ )
        close();
}

void CameraStreamReader::close()
{
    {
        QWriteLocker g(&rwLock_);
        shutdown_ = true;
    }
    wait();

    QWriteLocker g(&rwLock_);
    active_.reset();
    if( cap_->isOpened() )
        cap_->release();
}

void CameraStreamReader::run()
{
    bool shutdownFlag = false;

    Mat frame;
    while(1) 
    {
        // safe check for camera availability and for shutdown flag
        bool streaming = false;
        {
            QReadLocker g(&rwLock_);
            if( !(shutdownFlag = shutdown_) ) 
            {
                // camera works
                streaming = (cap_ && cap_->isOpened());
            }
            else
            {
                // exit thread
                break;
            }
        }

        if( streaming ) 
        {
            // try to read a frame
            try {
                frame.release();
                if( cap_->read(frame) && !frame.empty() ) 
                {
                    QWriteLocker g(&rwLock_);
                    // check shutting down after lock
                    if( shutdown_ )
                        break;

                    if( active_.isNull() ) {
                        active_.reset( new Mat(frame.size().width, frame.size().height, frame.type()) );
                        pictureSize_.rwidth() = frame.size().width;
                        pictureSize_.rheight() = frame.size().height;
                    }
                    frame.copyTo(*active_);
                }

                QReadLocker g(&rwLock_);
                if( !calibration_.isNull() && !active_.isNull() && !shutdown_ )
                {
                    calculateHistogram();
                    //calculateVectorOfView();
                }
            }
            catch(const cv::Exception& ex) 
            { qDebug() << "CameraStreamReader::run throws cv::Exception: " << ex.msg.c_str(); }
            catch(const std::exception& ex) 
            { qDebug() << "CameraStreamReader::run throws std::exception: " << ex.what(); }
            catch(...) 
            { qDebug() << "CameraStreamReader::run throws unhandled exception"; }
        }
        else
        {
            // 0.5 secs for idle
            QThread::msleep(500); 
        }
    }
}

bool CameraStreamReader::open()
{
    QWriteLocker g(&rwLock_);

    // default camera
    cap_.reset( new VideoCapture(0) );
    if( !cap_->isOpened() )
        return false;

    pictureSize_.rwidth() = cap_->get(CAP_PROP_FRAME_WIDTH);
    pictureSize_.rheight() = cap_->get(CAP_PROP_FRAME_HEIGHT);
    return true;
}

bool CameraStreamReader::readFrame(T2FrameShared& container)
{
    // QReadLocker g(&rwLock_);

    if( active_.isNull() || active_->empty() )
        return false;

    try {
        container->copyFrom(*active_);
        if( calibration_.isNull() )
            calibration_.reset( new CalibrateIt(container) );
        return true;
    }
    catch(...) {}
    return false;
}

void CameraStreamReader::calculateVectorOfView()
{
    calibration_->findRotation(vectorOfView_);
}

void CameraStreamReader::calculateHistogram()
{
    if( histoAccuracy_.isEmpty() || histoRGB_ == eNone ) 
        return;

    eBGR channels[3] = {eBlue, eGreen, eRed};
    for(int i = 0; i < 3; ++i)
    {
        if( histoRGB_ & channels[i] ) 
        {
            // find safely
            //QReadLocker g(&rwLock_);
            if( histograms_[i].size() != histoAccuracy_.width() )
                histograms_[i].resize(histoAccuracy_.width());
            calibration_->findHistogram(histograms_[i], channels[i], histoAccuracy_.height());
        }
    }
}

QVector<qreal> CameraStreamReader::calibrationView()
{
    QReadLocker g(&rwLock_);
    return vectorOfView_;
}


QVector<qint16> CameraStreamReader::blueHistogram()
{
    //QReadLocker g(&rwLock_);
    return histograms_[0];
}

QVector<qint16> CameraStreamReader::greenHistogram()
{
    //QReadLocker g(&rwLock_);
    return histograms_[1];
}

QVector<qint16> CameraStreamReader::redHistogram()
{
    //QReadLocker g(&rwLock_);
    return histograms_[2];
}
