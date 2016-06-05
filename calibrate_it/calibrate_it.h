#ifndef __calibrate_it_h__
#define __calibrate_it_h__

#include <qopengl.h>
#include <QSharedPointer>
#include <QVector>
#include <QVector3D>

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>

////////////////////////////////////////////////////////////
enum eBGR 
{
    eNone  = 0,
    eBlue  = 1,
    eGreen = 2,
    eRed   = 4,
};

////////////////////////////////////////////////////////////
class T2FrameContainer;
typedef QSharedPointer<T2FrameContainer> T2FrameShared;

class CalibrateIt
{
public:
    CalibrateIt(T2FrameShared& container);
    ~CalibrateIt();

    // Find 3-dims vector of the turn angle of camera between previous and current frames
    bool findRotation(QVector<qreal>& vc);

    // Find n-dims vector for histogram points
    // @param bgr is eBGR enumeration value of channel color
    // @maxh maximum height of historgram bin (used for vector normalization)
    void findHistogram(QVector<qint16>& vñ, eBGR bgr, qint16 maxh);

private:
    bool findChessboardEtalon();
    QVector<qreal> calibrateProcess();

private:
    // first frame
    cv::Mat grayMatLeft_;
    // next frame
    cv::Mat grayMatRight_;

    cv::Mat histChannels_[4];
    T2FrameShared container_;
};

#endif // __calibrate_it_h__
