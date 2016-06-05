#include "calibrate_defines.h"
#include "camera_widget.h"
#include "camera_stream_reader.h"
#include "movie2stereo_matcher.h"

#include <QtWidgets>

using namespace cv;
using namespace cv::ogl;

////////////////////////////////////////////////////////////////
CameraWidget::CameraWidget(QOpenGLContext* sharedGLContext)
    : QOpenGLWindow(sharedGLContext),
    initialized_(false),
    cameraArea_(0),
    cube_(0),
    reader_(NULL)
{}

CameraWidget::~CameraWidget()
{
    reader_ = NULL;
    initialized_ = false;
}

bool CameraWidget::start(CameraStreamReader* reader)
{
    if( !initialized_ )
        initializeGL();

    reader_ = reader;
    timer_.start(CAMERA_REFRESH_TIME, this);
    stereoMatcher_.reset(new M2StereoMatcher(this,this));
    return true;
}

void CameraWidget::stop()
{
    timer_.stop();
    frameCont_->clear();
    stereoMatcher_->stop();
    reader_ = NULL;
}

void CameraWidget::initializeGL()
{
    initializeOpenGLFunctions();

    // create storage for 15 last frames
    if( frameCont_.isNull() ) 
        frameCont_.reset(new T2FrameContainer(this,10));

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_BLEND);
    
    createCameraArea();
    createMatchingArea();
    createCube(0.1);

    initialized_ = true;
}

void CameraWidget::resizeGL(int w, int h)
{
    if( !initialized_ ) return;

    glViewport(0, 0, w, h);

    // set perspective
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0f, 1.0f, -1.0f, 1.0f, 0.7f, 7.0f);
    glMatrixMode(GL_MODELVIEW);
}

void CameraWidget::paintGL()
{
    if( !initialized_ ) return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor4f (1.0f, 1.0f, 1.0f, 0.5f);

    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);

    if( reader_ == NULL ) 
    {
        if( isValid() ) {
            glLoadIdentity();
            context()->swapBuffers(this);
        }
        return;
    }

    if( !reader_->readFrame(frameCont_) )
        return;

    if( !stereoMatcher_->isRunning() )
        stereoMatcher_->start();

    // binds 2D active texture and enters an internal critical section 

    bool cameraBind = false;
    if( cameraArea_ && (cameraBind = frameCont_->bind()) ) 
    {
        orthogonalBegin();
        glCallList(cameraArea_);
        orthogonalEnd();
    }

    if( matchingArea_ && stereoMatcher_->bind(*frameCont_->getLast()) ) 
    {
        orthogonalBegin();
        glCallList(matchingArea_);
        orthogonalEnd();
    }

    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
        // move backof on -1 logical units
        static qreal angle = 0;
        angle = (angle == 360 ? 0 : angle+1);

        glTranslatef(-1.0f, -0.15f, -0.95f);
        glRotatef(angle, 0.8f, 0.5f, 0.0f); 

        if( cube_ )
            glCallList(cube_);
    glPopMatrix();
    

    // save previous state
    QSurface* prevSurface = context()->surface();

    // set current window thread to active
    context()->makeCurrent(this);
    context()->swapBuffers(this);

    // restore the saved previously state if it actually
    if( prevSurface != this )
        context()->makeCurrent(prevSurface);

    // release critical sections of texures binding
    if( cameraBind )
        frameCont_->unbind();
}

bool CameraWidget::event(QEvent* e)
{
    switch(e->type()) 
    {
    case QEvent::Timer:
        paintGL();
        return true;
    case QEvent::UpdateRequest:
        if( initialized_ )
            context()->makeCurrent(this);
        break;
    }
    return QOpenGLWindow::event(e);
}

void CameraWidget::createCameraArea()
{
    GLint n  = glGenLists(1);

    glNewList(n, GL_COMPILE);
    glBegin(GL_QUADS);
        glTexCoord2d(0, 0); glVertex3f(-1,  1, 1);
        glTexCoord2d(1, 0); glVertex3f( 1,  1, 1);
        glTexCoord2d(1, 1); glVertex3f( 1, 0, 1);
        glTexCoord2d(0, 1); glVertex3f(-1, 0, 1);
    glEnd();

    glEndList();

    if( cameraArea_ )
        glDeleteLists(cameraArea_,1);
    cameraArea_ = n;
}

void CameraWidget::createMatchingArea()
{
    GLint n  = glGenLists(1);

    glNewList(n, GL_COMPILE);
    glBegin(GL_QUADS);
        glTexCoord2d(0, 0); glVertex3f(-1,  0, 1);
        glTexCoord2d(1, 0); glVertex3f(1,  0, 1);
        glTexCoord2d(1, 1); glVertex3f(1, -1, 1);
        glTexCoord2d(0, 1); glVertex3f(-1, -1, 1);
    glEnd();
    glEndList();

    if( matchingArea_ )
        glDeleteLists(matchingArea_,1);
    matchingArea_ = n;
}

void CameraWidget::createCube(qreal size)
{
    GLuint n = glGenLists(1);
    glNewList(n, GL_COMPILE);

    glBlendFunc(GL_ONE, GL_DST_COLOR);
    glColor3f(0.3f, 1.0f, 0.0f);

    for(qreal i = -size; i <= size; i+=0.1)
    {
        glBegin(GL_LINES);
            glVertex3f(-size, 0, i);
            glVertex3f(size, 0, i);
            glVertex3f(i, 0, -size);
            glVertex3f(i, 0, size);

            glVertex3f(0, -size, i);
            glVertex3f(0, size, i);
            glVertex3f(0, i, -size);
            glVertex3f(0, i, size);

            glVertex3f(-size, i, 0);
            glVertex3f(size, i, 0);
            glVertex3f(i, -size, 0);
            glVertex3f(i, size, 0);
        glEnd();
    }

    glEndList();

    if( cube_ )
        glDeleteLists(cube_,1);
    cube_ = n;
}

void CameraWidget::orthogonalBegin() 
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
}

void CameraWidget::orthogonalEnd()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
