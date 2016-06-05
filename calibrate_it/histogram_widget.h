#ifndef __histogram_widget_h__
#define __histogram_widget_h__

#include "qcustomplot.h"

////////////////////////////////////////////////////////////
class CameraStreamReader;

class HistogramWidget : public QCustomPlot
{
public:
    HistogramWidget(QWidget* parent);
    ~HistogramWidget();

    void start(CameraStreamReader* reader);
    void stop();

protected:
    void resizeEvent(QResizeEvent* e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent* e) Q_DECL_OVERRIDE;

private:
    CameraStreamReader* reader_;
    QBasicTimer timer_;
};

#endif // __histogram_widget_h__
