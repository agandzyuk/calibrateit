#include "calibrate_defines.h"
#include "histogram_widget.h"
#include "camera_stream_reader.h"
#include "calibrate_it.h"

#include <QtWidgets>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>


using namespace cv;

HistogramWidget::HistogramWidget(QWidget* parent)
    : QCustomPlot(parent),
    reader_(NULL)
{
}

HistogramWidget::~HistogramWidget()
{
    stop();
}

void HistogramWidget::stop()
{
    timer_.stop();
}

void HistogramWidget::start(CameraStreamReader* reader)
{
    reader_ = reader;
    timer_.start(CAMERA_REFRESH_TIME*2,this);
}

void HistogramWidget::resizeEvent(QResizeEvent* e)
{
    QCustomPlot::resizeEvent(e);

    bool visible = (reader_ != NULL);
    xAxis->setVisible(visible);
    yAxis->setVisible(visible);

    if( visible ) 
    {
        // brigtness 0.0 - 1.0f
        xAxis->setRange(0, 1);
        qint32 picPixAmount = reader_->pictureSize().width()*reader_->pictureSize().height();
        picPixAmount *= 1.2f; // multiple 1.2 to left a space on top
        // intencity 0 - numOfPixels
        yAxis->setRange(0, picPixAmount); 
        reader_->setHistogrammAccuracy(QSize(xAxis->axisRect()->width()*0.65f, yAxis->axisRect()->height()), 
                                       eRed|eBlue|eGreen);
    }
    replot();
}

void HistogramWidget::timerEvent(QTimerEvent* e)
{
    Q_UNUSED(e);

    if( reader_ == NULL )
        return;

    if( 0 == graphCount() )
    {
        xAxis->setLabel("brightness");
        yAxis->setLabel("intensity (pixels number)");
        if( 0 == graphCount() ) 
        {
            addGraph();
            addGraph();
            addGraph();
        }
    }

    for(qint8 c = 0; c < 3; c++)
    {
        QVector<qint16> vc;
        if( c == 0 ) {
            graph(0)->setPen(QPen(QColor(Qt::blue)));
            vc = reader_->greenHistogram();
        }
        else if( c == 1) {
            graph(1)->setPen(QPen(QColor(Qt::green)));
            vc = reader_->blueHistogram();
        }
        else if( c == 2 ) {
            graph(2)->setPen(QPen(QColor(Qt::red)));
            vc = reader_->redHistogram();
        }

        if( !vc.empty() ) 
        {
            qint32 picPixAmount = reader_->pictureSize().width() * reader_->pictureSize().height();
            qint32 normalizedValMaxAmount = reader_->histogramAccuracy().height();

            qreal hd = picPixAmount/normalizedValMaxAmount;
            qreal wd = reader_->histogramAccuracy().width();

            QVector<double> x,y;
            for(qint16 i = 0; i < vc.size(); i++) {
                x.push_back(qreal(i)/wd); 
                y.push_back(qreal(vc[i])*hd);
            }
            graph(c)->setData(x,y);
        }
    }
    replot();
}
