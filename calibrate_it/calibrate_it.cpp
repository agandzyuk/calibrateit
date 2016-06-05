#include "calibrate_defines.h"
#include "calibrate_it.h"
#include "texture2frame_container.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d_c.h>

// 5x4 aspect ration from camera matrix
#define CAMERA_ASPECT_RATIO     (0.8f)

// simplified image width to inrcease calibration speed
#define SIMPLIFIED_WIDTH        (100)                               // 100 pixes
#define SIMPLIFIED_HEIGHT       ((int)(SIMPLIFIED_WIDTH*CAMERA_ASPECT_RATIO))    // calc with aspect

using namespace cv;

extern void showTestImage(const cv::Mat& bits);

///////////////////////////////////////////////////////////////////////
CalibrateIt::CalibrateIt(T2FrameShared& container)
    : container_(container),
    grayMatLeft_(SIMPLIFIED_WIDTH, SIMPLIFIED_HEIGHT, CV_8UC1),
    grayMatRight_(SIMPLIFIED_WIDTH, SIMPLIFIED_HEIGHT, CV_8UC1)
{}

CalibrateIt::~CalibrateIt()
{}

bool CalibrateIt::findRotation(QVector<qreal>& vc)
{
    if( container_.isNull() || container_->size() < 2 )
        return false;

    bool fresult = false;
    const Mat *curFrame, *prevFrame;
    if( (curFrame = container_->getN(0)) && (prevFrame = container_->getN(1)) ) 
    {
        // convert to gray
        if( prevFrame->channels() > 1 )
            cv::cvtColor(*prevFrame, grayMatLeft_, CV_BGR2GRAY);
        if( curFrame->channels() > 1 )
            cv::cvtColor(*curFrame, grayMatRight_, CV_BGR2GRAY);

        // convert to binary
        CvMat cpPtr = grayMatLeft_;
        cvInRangeS(&cpPtr, cvScalar(40), cvScalar(150), &cpPtr);
        cpPtr = grayMatRight_;
        cvInRangeS(&cpPtr, cvScalar(40), cvScalar(150), &cpPtr);
        cvSaveImage("1r.jpg", &CvMat(grayMatRight_));
        showTestImage(grayMatRight_);
    }
    return fresult;
}

/*
bool CalibrateIt::findChessboardEtalon()
{
    int tempPointCount = nCornersPerImage_*2;
    if( pLatestPoints_ == NULL )
        pLatestPoints_ = (CvPoint2D32f*)cvAlloc(tempPointCount*2*sizeof(CvPoint2D32f));

    Mat& refMat = *container_->getLast();

    // check the pointer and size, create new matrix if needed
    if( grayMat_.isNull() ||
        (grayMat_->size().width != refMat.size().width) || 
        (grayMat_->size().height != refMat.size().height) )
    {
        grayMat_.reset( new Mat(refMat.size().width, refMat.size().height, CV_8UC1) );
    }

    // create grayscale copy
    cv::cvtColor(refMat, *grayMat_.data(), CV_BGR2GRAY);

    int pointCount = 0;

    CvMat matCpy( *grayMat_.data() );
    bool found = (bool)cvFindChessboardCorners( &matCpy,
                                                CvSize(NUMBER_OF_COLUMNS-1,NUMBER_OF_ROWS-1),
                                                pLatestPoints_, 
                                                &pointCount, 
                                                CV_CALIB_CB_ADAPTIVE_THRESH );
    if( found )
    {
        cvFindCornerSubPix( &matCpy, 
                            pLatestPoints_, 
                            pointCount, 
                            cvSize(5,5), 
                            cvSize(-1,-1),
                            cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,10,0.1) );
    }

    return found;
}
*/

/*
( const CvMat* object_points, const CvMat* image_points1,
                               const CvMat* image_points2, const CvMat* npoints,
                               CvMat* camera_matrix1, CvMat* dist_coeffs1,
                               CvMat* camera_matrix2, CvMat* dist_coeffs2,
                               CvSize image_size, CvMat* R, CvMat* T,
                               CvMat* E CV_DEFAULT(0), CvMat* F CV_DEFAULT(0),
                               int flags CV_DEFAULT(CV_CALIB_FIX_INTRINSIC),
                               CvTermCriteria term_crit CV_DEFAULT(cvTermCriteria(
                                   CV_TERMCRIT_ITER+CV_TERMCRIT_EPS,30,1e-6)) );
*/

QVector<qreal> CalibrateIt::calibrateProcess()
{
    QVector<qreal> rotation;

    QVector<QVector<Point3f>> objpt(2);
    QVector<QVector<Point2f>> imgpt1(2);
    QVector<QVector<Point2f>> imgpt2(2);


    //cvStereoCalibrate(&vCurFram, &vPrevFram, pF, imgSize, &_H1, &_H2, threshold) > 0;
    return rotation;
}

void CalibrateIt::findHistogram(QVector<qint16>& vc, eBGR bgr, qint16 maxh)
{
    if( container_.isNull() || (0 == container_->size()) ) 
        return;

    int histoBins = vc.size();
    if( histoBins < 3 ) {
        Q_ASSERT_X(histoBins > 2,"CalibrateIt::findHistogram","3 is minimal count of histohram bins");
        return;
    }

    qint8 idx = 0;
    switch(bgr) {
    case eBlue: idx = 0; break;
    case eRed: idx = 1; break;
    case eGreen: idx = 2; break;
    default:
        Q_ASSERT_X(false,"CalibrateIt::findHistogram","invalid parameter value ('bgr')");
        return;
    }

    float range[] = {0.0f, 256.0f};
    const float* histRange = {range}; 

    if( histChannels_[idx].empty() ) 
    {
        Mat* lastFrame = container_->getLast();
        if( lastFrame == NULL )
            return;
        cv::split(*lastFrame, histChannels_);
    }

    Mat cvHisto;
    calcHist(&histChannels_[idx], 1, 0, Mat(), cvHisto, 1, &histoBins, &histRange);
    normalize(cvHisto, cvHisto, 0, maxh, NORM_MINMAX);

    qint16 prev2[2] = {0};
    for(qint16 c = 0; c < histoBins; c++) 
    {
        qint16 val = cvRound(cvHisto.at<float>(c));

        // smoothing of the unexpected dropping to zero
        if( prev2[0]*prev2[1] > 0 && val == 0 ) {
            vc[c] = prev2[1];
        }
        else {
            vc[c] = val;
        }

        prev2[0] = prev2[1];
        prev2[1] = val;
    }

    histChannels_[idx].release();
}
