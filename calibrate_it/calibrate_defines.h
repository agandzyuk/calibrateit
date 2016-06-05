#ifndef __calibrate_defines_h__
#define __calibrate_defines_h__

#include <QString>
#include <QPoint>

#define CAMERA_REFRESH_TIME 20 // ms

////////////////////////////////////////////////////////////
QT_BEGIN_NAMESPACE;
class QFont;
QT_END_NAMESPACE;

////////////////////////////////////////////////////////////
struct Globals
{
    static void Init();

    static QPoint desktop;
    static QFont* originFont;
    static QFont* smallFont;
    static QFont* boldFont;
};

#endif // __calibrate_defines_h__
