#ifndef __camera_widget_h__
#define __camera_widget_h__


#include "texture2frame_container.h"

#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QBasicTimer>
#include <opencv2/core/opengl.hpp>

////////////////////////////////////////////////////////////
class CameraStreamReader;
class T2FrameContainer;
class M2StereoMatcher;

class CameraWidget : public QOpenGLWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    CameraWidget(QOpenGLContext* sharedGLContext);
    ~CameraWidget();

    bool start(CameraStreamReader* reader);
    void stop();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    bool event(QEvent* e) Q_DECL_OVERRIDE;

private slots:

private:
    void orthogonalBegin();
    void orthogonalEnd();
    void createCameraArea();
    void createMatchingArea();
    void createCube(qreal size);

private:
    bool initialized_;
    CameraStreamReader* reader_;
    T2FrameShared   frameCont_;
    QSharedPointer<M2StereoMatcher> stereoMatcher_;

    QBasicTimer timer_;
    QMatrix4x4 projection;

    GLuint cameraArea_;
    GLuint matchingArea_;
    GLuint cube_;
};

#endif // __camera_widget_h__
