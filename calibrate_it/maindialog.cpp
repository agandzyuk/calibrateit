#include "calibrate_defines.h"
#include "maindialog.h"
#include "camera_widget.h"
#include "histogram_widget.h"
#include "camera_stream_reader.h"

#include <QtWidgets>
#include <QtGui>

////////////////////////////////////////////////////////////
MainDialog::MainDialog(QObject* parent)
    : streamingEnabled_(false)
{
    Q_UNUSED(parent);
    setWindowTitle(tr("CalibrateIt"));

    QHBoxLayout* hlayout = new QHBoxLayout(this);

    hlayout->addWidget( createViewGroup() );
    hlayout->addWidget( createHistGroup() );
    setLayout(hlayout);

    resize(Globals::desktop.x()*0.8, Globals::desktop.y()*0.65);
}

MainDialog::~MainDialog()
{
    if( streamingEnabled_ ) {
        histogramWidget_->stop();
        cameraWidget_->stop();
        reader_->close();
    }
}

QWidget* MainDialog::createViewGroup()
{
    QGroupBox* groupBox = new QGroupBox(tr("Camera View"),this);
    QVBoxLayout* vlayout = new QVBoxLayout();

    sharedGLContext_.reset( new QOpenGLContext(this) );
    if( !sharedGLContext_->supportsThreadedOpenGL() )
    { throw std::exception("OpenGL multithreading is disabled"); }

    if( !sharedGLContext_->create() )
    { throw std::exception("Cannot create shared OpenGL context"); }

    cameraWidget_ = new CameraWidget(sharedGLContext_.data());
    if( cameraWidget_ == NULL )
    { throw std::exception("Cannot create Camera's GL widget"); }

    vlayout->addWidget( QWidget::createWindowContainer(cameraWidget_) );

    btnView_ = new QPushButton(tr("Start Cam"),this);
    QObject::connect(btnView_, SIGNAL(clicked()), this, SLOT(onView()));
    vlayout->addWidget(btnView_, 5, Qt::AlignRight);

    groupBox->setLayout(vlayout);

    return groupBox;
}


QWidget* MainDialog::createHistGroup()
{
    QGroupBox* groupBox = new QGroupBox(tr("Histogram View"),this);
    QVBoxLayout* vlayout = new QVBoxLayout();

    vlayout->addWidget(histogramWidget_ = new HistogramWidget(this));
    histogramWidget_->hide();

    btnClose_ = new QPushButton(tr("Close"),this);
    QObject::connect(btnClose_, SIGNAL(clicked()), this, SLOT(close()));
    vlayout->addWidget(btnClose_, 1, Qt::AlignBottom|Qt::AlignRight);

    groupBox->setLayout(vlayout);
    return groupBox;
}

void MainDialog::onView()
{
    if( reader_.isNull() ) 
        reader_.reset(new CameraStreamReader(this));

    if( streamingEnabled_ ) 
    {
        cameraWidget_->stop();
        histogramWidget_->stop();
        reader_->close();
        reader_.reset();

        btnView_->setText(tr("Start Cam"));
        streamingEnabled_ = false;
    }
    else {
        if( reader_->open() ) 
        {
            cameraWidget_->start( reader_.data() );
            btnView_->setText(tr("Stop"));
            streamingEnabled_ = true;

            if( histogramWidget_->isHidden() )
            {
                histogramWidget_->start(reader_.data());
                QLayout* rVLayour = layout()->itemAt(1)->widget()->layout();
                if( rVLayour )  {
                    rVLayour->itemAt(1)->widget()->setFixedSize(btnClose_->size());
                    rVLayour->itemAt(1)->setAlignment(Qt::AlignRight);
                    rVLayour->itemAt(0)->widget()->show();
                    rVLayour->itemAt(0)->widget()->resize(cameraWidget_->size());
                }
            }
            else
                histogramWidget_->start(reader_.data());
        }
        else
            QMessageBox::warning(this, "Error", "OS installed devices doesn't contain any Camera drivers", "OK");
    } 
}

void MainDialog::showEvent(QShowEvent* e)
{
    QDialog::showEvent(e);
}
