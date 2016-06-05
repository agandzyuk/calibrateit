#ifndef __movie2stereo_matcher_h__
#define __movie2stereo_matcher_h__

#include "texture2frame_container.h"

#include <QThread>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <QQueue>

#include <opencv2/core.hpp>

////////////////////////////////////////////////////////////
QT_BEGIN_NAMESPACE;
class QOpenGLFunctions;
QT_END_NAMESPACE;

////////////////////////////////////////////////////////////
class M2StereoMatcher : public QThread
{
public:
    M2StereoMatcher(QObject* parent, QOpenGLFunctions* glfx);
    ~M2StereoMatcher();

    void start();
    void stop();

    // binding the calculated frame from container
    bool bind(const cv::Mat& frame);

protected:
    void run() Q_DECL_OVERRIDE;
    void calculate();

private:
    T2FrameShared container_;
    QSharedPointer<cv::Mat> front_;
    QSharedPointer<cv::Mat> back_;
    QSharedPointer<cv::Mat> disparity_;

    QReadWriteLock rwLock_;
    bool shutdown_;
    QOpenGLFunctions* glfx_;
};

#endif // __movie2stereo_matcher_h__
