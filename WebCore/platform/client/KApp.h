
#ifndef _KApp_H_
#define _KApp_H_

#include "wtf/Platform.h"
//#include "Text/QFont.h"
//#include "Client/QClipboard.h"
//#include "Painting/QGraphicsSystem_p.h"

class QClipboard;
class QPalette;
class QGraphicsSystem;
class QFont;
class QPixmap;
class QThread;

#define kApp KApp::instance()

#ifndef QT_NO_CLIPBOARD
extern QClipboard *kd_clipboard;
#endif

extern double qt_fontsmoothing_gamma;
extern bool qt_cleartype_enabled;
extern bool qt_win_owndc_required; // CS_OWNDC is required if we use the GL graphicssystem as default

class KApp
{
public:
    enum KeyPlatform {
        KB_Win = 1,
        KB_Mac = 2,
        KB_X11 = 4,
        KB_KDE = 8,
        KB_Gnome = 16,
        KB_CDE = 32,
        KB_S60 = 64,
        KB_All = 0xffff
    };

    static unsigned int currentPlatform();

    static KApp* instance();

    static QClipboard* clipboard();

    static void beep();

    static QPalette* palette();

    //static QFont font();
    //static QFont font(const char *className);

    static QFont* font();
    static QFont* font(const char *className);

    static void setFont(const QFont &font, const char *className);

#ifndef QT_NO_WHEELEVENT
    static void setWheelScrollLines(int);
    static int wheelScrollLines();
#endif

//     static QGraphicsSystem* graphics_system;
//     static bool runtime_graphics_system;
//     static QString graphics_system_name;
//     static QPalette* app_pal;
//     static QThread* theMainThread;
//     static void* AppInst;

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    QPixmap *move_cursor;
    QPixmap *copy_cursor;
    QPixmap *link_cursor;
#endif
#if defined(Q_WS_WIN)
    QPixmap *ignore_cursor;
#endif
    //QPixmap* getPixmapCursor(Qt::CursorShape cshape);

protected:
    friend class QClipboard;

    KApp();

    static KApp* m_pInst;

};

#endif // _KApp_H_