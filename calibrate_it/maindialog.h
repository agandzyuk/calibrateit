#ifndef __maindialog_h__
#define __maindialog_h__

#include <QDialog>

////////////////////////////////////////////////////////////
QT_BEGIN_NAMESPACE;
class QOpenGLContext;
QT_END_NAMESPACE;


class CameraWidget;
class HistogramWidget;
class CameraStreamReader;

////////////////////////////////////////////////////////////
class MainDialog : public QDialog
{
    Q_OBJECT

public:
    MainDialog(QObject* parent);
    ~MainDialog();

protected:
    QWidget* createViewGroup();
    QWidget* createHistGroup();
    void showEvent(QShowEvent* e) Q_DECL_OVERRIDE;

protected slots:
    void onView();
    
private:
    CameraWidget* cameraWidget_;
    HistogramWidget* histogramWidget_;
    QPushButton*  btnView_;
    QPushButton*  btnClose_;
    QSharedPointer<QOpenGLContext> sharedGLContext_;
    QSharedPointer<CameraStreamReader> reader_;
    bool streamingEnabled_;
};

#endif // __maindialog_h__
