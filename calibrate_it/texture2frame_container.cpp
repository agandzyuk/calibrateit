#include "texture2frame_container.h"

///////////////////////////////////////////////////////////////////
T2FrameContainer::T2FrameContainer(QOpenGLFunctions* glfx, qint16 maxSize) 
    : bindLock_(new QMutex()), 
    glfx_(glfx),
    max_(maxSize), 
    size_(0)
{}

T2FrameContainer::~T2FrameContainer()
{
    clear();
    delete bindLock_;
}

void T2FrameContainer::clear()
{ 
    size_ = 0;

    if( !bindLock_->tryLock() )
        Q_ASSERT_X(false, "T2FrameContainer::~T2FrameContainer","unbind() was not called after binding");
    bindLock_->unlock();

    // free allocated textures
    while( !empty() ) {
        glfx_->glDeleteTextures(1,&back().first);
        pop_back();
    }
}

void T2FrameContainer::copyFrom(const cv::Mat& cvFrame)
{
    // may be in binding state
    //QMutexLocker g(bindLock_);
    if(size_ == max_) 
    {
        // move texture object as in ring
        push_front(back());
        pop_back();
    }
    else
    {
        push_front(Texture2Frame());
        glfx_->glGenTextures(1, &front().first);
        size_++;
    }

    bool newTx = false;
    cv::Mat* frame = front().second.data();
    if( (frame==NULL) || (cvFrame.cols!=frame->cols) || (cvFrame.rows!=frame->rows) ) 
    {
        newTx = true;
        /*
        // recreate texture because size is changed
        if( frame ) {
            glfx_->glDeleteTextures(1, &front().first);
            glfx_->glGenTextures(1, &front().first);
        }
        */
        front().second.reset(new cv::Mat(cvFrame.rows, cvFrame.cols, cvFrame.type()));
        frame = front().second.data();
    }

    // convert to RGBA if need
    if( cvFrame.channels() == 3 ) 
        cvtColor(cvFrame, *frame, CV_BGR2RGBA);
    else if( cvFrame.type() == CV_8UC1 ) 
        cvtColor(cvFrame, *frame, CV_GRAY2RGBA);
    else
        cvFrame.copyTo(*frame);

    cv::Size asize = frame->size();
    frameSize_.rwidth() = asize.width;
    frameSize_.rheight() = asize.height;

    // enable 2D textures
    glfx_->glBindTexture(GL_TEXTURE_2D,0);

    glfx_->glBindTexture(GL_TEXTURE_2D,front().first);

    // enable one-byte alignment for pixels copying
    glfx_->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // create texture
    if( newTx )
        glfx_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, asize.width, asize.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame->data);
    glfx_->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, asize.width, asize.height, GL_RGBA, GL_UNSIGNED_BYTE, frame->data);

    glfx_->glGenerateMipmap(GL_TEXTURE_2D);
}

bool T2FrameContainer::bind()
{
    if( empty() ) 
        return false;

    bindLock_->lock();
    glfx_->glBindTexture(GL_TEXTURE_2D, front().first);
    return true;
}

void T2FrameContainer::unbind()
{
    bindLock_->unlock();
}
