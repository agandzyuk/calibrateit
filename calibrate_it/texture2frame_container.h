#ifndef __texture2frame_container_h__
#define __texture2frame_container_h__

#include <QOpenGLFunctions>
#include <opencv2/imgproc.hpp>
#include <QMutex>
#include <QSize>

////////////////////////////////////////////////////////////
struct Texture2Frame : public QPair<GLuint,QScopedPointer<cv::Mat>>
{
    inline Texture2Frame() {
        first = 0;
        second.reset(NULL);
    }
    inline Texture2Frame(const Texture2Frame& rval) {
        first = rval.first;
        if( !rval.second.isNull() )
            second.reset( new cv::Mat(*rval.second) );
    }
    inline Texture2Frame& operator=(const Texture2Frame& rval) {
        first = rval.first;
        if( !rval.second.isNull() )
            second.reset( new cv::Mat(*rval.second) );
        return *this;
    }
};

////////////////////////////////////////////////////////////
// Container class for frames and its textures binding
// It represents queue with fixed maximum size, 
// so newest element will be pushed at head and oldest element be poped from tail
// (conditionaly when size of queue equals to container max size)
class T2FrameContainer : private QList<Texture2Frame>
{
    typedef QList<Texture2Frame> BaseT;

public:
    // @maxSize: max number of frames in container wich can be stored
    T2FrameContainer(QOpenGLFunctions* glfx, qint16 maxSize);
    virtual ~T2FrameContainer();

    // copy frame from cv::Mat buffer and push it to container
    void copyFrom(const cv::Mat& cvFrame);

    // binding the last textured frame from container
    // it sets critical seciton of binding, so call unbind() after section should be closed
    //@returns false if container is empty
    bool bind();

    // unbinding and release internal mutex
    void unbind();

    // get active frame width
    inline qint16 width() const;

    // get active frame heigth
    inline qint16 height() const;

    // get last recieved frame
    inline cv::Mat* getLast() const;

    // get frame by index in container
    // @index: 0 - last, 1 - prev, 2 - behind prev, etc
    inline cv::Mat* getN(int index) const;

    // public for QList::size
    inline qint16 size() const;

    // free resources
    void clear();

protected:
    // reserve textures
    void reserve();

private:
    qint16 max_;
    qint16 size_;
    QSize frameSize_;
    QOpenGLFunctions* glfx_;
    QMutex* bindLock_;
};

typedef QSharedPointer<T2FrameContainer> T2FrameShared;

/////////////////////////////////////////////////////////////////////
// Class inlines
inline qint16 T2FrameContainer::size() const
{ return empty() ? 0 : BaseT::size(); }

inline qint16 T2FrameContainer::width() const
{ return frameSize_.width(); }

inline qint16 T2FrameContainer::height() const
{ return frameSize_.height(); }

inline cv::Mat* T2FrameContainer::getLast() const {
    cv::Mat* img = empty() ? NULL : front().second.data();
    return img;
}

inline cv::Mat* T2FrameContainer::getN(int index) const {
    BaseT::ConstIterator It = cbegin();
    for(int c = 0; It != cend() && c < index; ++c, ++It);
    Q_ASSERT_X(It != cend(), "T2FrameContainer::getN", "Returning image is <null>");
    return (It != cend() ? It->second.data() : NULL);
}

#endif // __texture2frame_container_h__
