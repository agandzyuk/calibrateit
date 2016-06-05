#ifndef __camera_stream_reader_h__
#define __camera_stream_reader_h__

#include <QThread>
#include <QReadWriteLock>
#include <QSize>
#include <QVector>
#include <QPointf>
#include <QVector3D>

////////////////////////////////////////////////////////////
namespace cv {
    class VideoCapture;
    class Mat;
}

////////////////////////////////////////////////////////////
class T2FrameContainer;
typedef QSharedPointer<T2FrameContainer> T2FrameShared;
class CalibrateIt;

class CameraStreamReader : public QThread
{
public:
    CameraStreamReader(QObject* parent);
    ~CameraStreamReader();

    bool open();
    void close();

    bool readFrame(T2FrameShared& container);

    QVector<qint16> blueHistogram();
    QVector<qint16> greenHistogram();
    QVector<qint16> redHistogram();
    QVector<qreal> calibrationView();

    inline const QSize& pictureSize() const
    { return pictureSize_; }

    inline const QSize& histogramAccuracy() const
    { return histoAccuracy_; }

    inline void setHistogrammAccuracy(const QSize& acc, quint8 ergb) {
        histoAccuracy_ = acc; 
        histoRGB_ = ergb;
    }

protected:
    void run() Q_DECL_OVERRIDE;

    void calculateHistogram();
    void calculateVectorOfView();

private:
    QScopedPointer<CalibrateIt> calibration_;
    QScopedPointer<cv::VideoCapture> cap_;
    QScopedPointer<cv::Mat> active_;
    QSize pictureSize_;

    QReadWriteLock rwLock_;
    bool shutdown_;

    QVector<qreal>  vectorOfView_;
    QVector<qint16> histograms_[3];

    QSize  histoAccuracy_;
    quint8 histoRGB_;
};

#endif // __camera_widget_h__
