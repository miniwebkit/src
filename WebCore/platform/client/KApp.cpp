#include "KApp.h"
#include <windows.h>
#include <painting/QColorMap.h>
#include <Client/QKeyMapper_p.h>
#include <client/QClipboard.h>
#include <Text/QFont.h>
#include <Painting/QGraphicsSystem_p.h>


#ifndef QT_NO_CLIPBOARD
QClipboard* kd_clipboard = 0;
#endif

qreal qt_fontsmoothing_gamma;
bool qt_cleartype_enabled;
bool qt_win_owndc_required; // CS_OWNDC is required if we use the GL graphicssystem as default

QGraphicsSystem* KApp::graphics_system = 0;
bool KApp::runtime_graphics_system = false;
KApp* KApp::m_pInst = 0;

QPalette* KApp::app_pal = 0;

QThread* KApp::theMainThread = 0;

bool qt_is_gui_used = true;

static QFont* app_font = 0;
static QFont* sys_font = 0;

void* KApp::AppInst = 0;

KApp* KApp::instance()
{
    if (!m_pInst) {
        m_pInst = new KApp;
    }

    return m_pInst;
}

extern void qt_win_read_cleartype_settings();
extern QFont qt_LOGFONTtoQFont(LOGFONT& lf, bool /*scale*/);
static void setSystemFont(const QFont &font, const char *className);

KApp::KApp()
{
    QKeyMapper::changeKeyboard();

    QColormap::initialize();
    QFont::initialize();

#ifndef Q_WS_WINCE
    HGDIOBJ stockFont = GetStockObject(DEFAULT_GUI_FONT);
#else
    HGDIOBJ stockFont = GetStockObject(SYSTEM_FONT);
#endif

    LOGFONT lf;
    GetObject(stockFont, sizeof(lf), &lf);
    QFont systemFont = qt_LOGFONTtoQFont(lf, true);

#ifndef Q_WS_WINCE
    if (systemFont.family() == QLatin1String("MS Shell Dlg")) {
        systemFont.setFamily(QLatin1String("MS Shell Dlg 2"));
    }
#endif
    setSystemFont(systemFont, "SystemFont");

    qt_win_read_cleartype_settings();
    qt_win_owndc_required = false;

    __asm int 3;
    m_pInst = 0;

    //extern void qt_win_initialize_directdraw();
    //qt_win_initialize_directdraw();
}

typedef QHash<QByteArray, QFont> FontHash;
static FontHash* app_fonts()
{
    static FontHash This;
    return &This;
}

void KApp::setFont(const QFont &font, const char *className)
{
    bool all = false;
    FontHash *hash = app_fonts();
    if (!className) 
        {__asm int 3;
            return;}

    if (app_font)
        delete app_font;
    app_font = new QFont(font);
    hash->insert(className, font);
}

QFont* KApp::font(const char *className)
{
    __asm int 3;
}

QFont* KApp::font()
{
    //__asm int 3; TODO_Weolar
    if (!app_font)
        app_font = new QFont(QLatin1String("Helvetica"));
    return app_font;
}

static void setSystemFont(const QFont &font, const char *className)
{
    if (sys_font)
        delete sys_font;

    sys_font = new QFont(font);

    KApp::setFont(font, className);
}

QPalette* KApp::palette()
{
    if (!app_pal)
        app_pal = new QPalette(Qt::black);
    return app_pal;
}

#ifndef QT_NO_CLIPBOARD
/*!
    Returns a pointer to the application global clipboard.

    \note The QApplication object should already be constructed before
    accessing the clipboard.
*/
QClipboard *KApp::clipboard()
{
    if (kd_clipboard == 0) {
        if (!kApp) {
            //qWarning("QApplication: Must construct a QApplication before accessing a QClipboard");
            return 0;
        }
        kd_clipboard = new QClipboard();
    }
    return kd_clipboard;
}


static const char * const move_xpm[] = {
    "11 20 3 1",
    ".        c None",
#if defined(Q_WS_WIN)
    "a        c #000000",
    "X        c #FFFFFF", // Windows cursor is traditionally white
#else
    "a        c #FFFFFF",
    "X        c #000000", // X11 cursor is traditionally black
#endif
    "aa.........",
    "aXa........",
    "aXXa.......",
    "aXXXa......",
    "aXXXXa.....",
    "aXXXXXa....",
    "aXXXXXXa...",
    "aXXXXXXXa..",
    "aXXXXXXXXa.",
    "aXXXXXXXXXa",
    "aXXXXXXaaaa",
    "aXXXaXXa...",
    "aXXaaXXa...",
    "aXa..aXXa..",
    "aa...aXXa..",
    "a.....aXXa.",
    "......aXXa.",
    ".......aXXa",
    ".......aXXa",
    "........aa."
};

#ifdef Q_WS_WIN
    /* XPM */
static const char * const ignore_xpm[] = {
        "24 30 3 1",
        ".        c None",
        "a        c #000000",
        "X        c #FFFFFF",
        "aa......................",
        "aXa.....................",
        "aXXa....................",
        "aXXXa...................",
        "aXXXXa..................",
        "aXXXXXa.................",
        "aXXXXXXa................",
        "aXXXXXXXa...............",
        "aXXXXXXXXa..............",
        "aXXXXXXXXXa.............",
        "aXXXXXXaaaa.............",
        "aXXXaXXa................",
        "aXXaaXXa................",
        "aXa..aXXa...............",
        "aa...aXXa...............",
        "a.....aXXa..............",
        "......aXXa.....XXXX.....",
        ".......aXXa..XXaaaaXX...",
        ".......aXXa.XaaaaaaaaX..",
        "........aa.XaaaXXXXaaaX.",
        "...........XaaaaX..XaaX.",
        "..........XaaXaaaX..XaaX",
        "..........XaaXXaaaX.XaaX",
        "..........XaaX.XaaaXXaaX",
        "..........XaaX..XaaaXaaX",
        "...........XaaX..XaaaaX.",
        "...........XaaaXXXXaaaX.",
        "............XaaaaaaaaX..",
        ".............XXaaaaXX...",
        "...............XXXX....."
};
#endif

/* XPM */
static const char * const copy_xpm[] = {
            "24 30 3 1",
            ".        c None",
            "a        c #000000",
            "X        c #FFFFFF",
#if defined(Q_WS_WIN) // Windows cursor is traditionally white
            "aa......................",
            "aXa.....................",
            "aXXa....................",
            "aXXXa...................",
            "aXXXXa..................",
            "aXXXXXa.................",
            "aXXXXXXa................",
            "aXXXXXXXa...............",
            "aXXXXXXXXa..............",
            "aXXXXXXXXXa.............",
            "aXXXXXXaaaa.............",
            "aXXXaXXa................",
            "aXXaaXXa................",
            "aXa..aXXa...............",
            "aa...aXXa...............",
            "a.....aXXa..............",
            "......aXXa..............",
            ".......aXXa.............",
            ".......aXXa.............",
            "........aa...aaaaaaaaaaa",
#else
            "XX......................",
            "XaX.....................",
            "XaaX....................",
            "XaaaX...................",
            "XaaaaX..................",
            "XaaaaaX.................",
            "XaaaaaaX................",
            "XaaaaaaaX...............",
            "XaaaaaaaaX..............",
            "XaaaaaaaaaX.............",
            "XaaaaaaXXXX.............",
            "XaaaXaaX................",
            "XaaXXaaX................",
            "XaX..XaaX...............",
            "XX...XaaX...............",
            "X.....XaaX..............",
            "......XaaX..............",
            ".......XaaX.............",
            ".......XaaX.............",
            "........XX...aaaaaaaaaaa",
#endif
            ".............aXXXXXXXXXa",
            ".............aXXXXXXXXXa",
            ".............aXXXXaXXXXa",
            ".............aXXXXaXXXXa",
            ".............aXXaaaaaXXa",
            ".............aXXXXaXXXXa",
            ".............aXXXXaXXXXa",
            ".............aXXXXXXXXXa",
            ".............aXXXXXXXXXa",
            ".............aaaaaaaaaaa"
};

/* XPM */
static const char * const link_xpm[] = {
                "24 30 3 1",
                ".        c None",
                "a        c #000000",
                "X        c #FFFFFF",
#if defined(Q_WS_WIN) // Windows cursor is traditionally white
                "aa......................",
                "aXa.....................",
                "aXXa....................",
                "aXXXa...................",
                "aXXXXa..................",
                "aXXXXXa.................",
                "aXXXXXXa................",
                "aXXXXXXXa...............",
                "aXXXXXXXXa..............",
                "aXXXXXXXXXa.............",
                "aXXXXXXaaaa.............",
                "aXXXaXXa................",
                "aXXaaXXa................",
                "aXa..aXXa...............",
                "aa...aXXa...............",
                "a.....aXXa..............",
                "......aXXa..............",
                ".......aXXa.............",
                ".......aXXa.............",
                "........aa...aaaaaaaaaaa",
#else
                "XX......................",
                "XaX.....................",
                "XaaX....................",
                "XaaaX...................",
                "XaaaaX..................",
                "XaaaaaX.................",
                "XaaaaaaX................",
                "XaaaaaaaX...............",
                "XaaaaaaaaX..............",
                "XaaaaaaaaaX.............",
                "XaaaaaaXXXX.............",
                "XaaaXaaX................",
                "XaaXXaaX................",
                "XaX..XaaX...............",
                "XX...XaaX...............",
                "X.....XaaX..............",
                "......XaaX..............",
                ".......XaaX.............",
                ".......XaaX.............",
                "........XX...aaaaaaaaaaa",
#endif
                ".............aXXXXXXXXXa",
                ".............aXXXaaaaXXa",
                ".............aXXXXaaaXXa",
                ".............aXXXaaaaXXa",
                ".............aXXaaaXaXXa",
                ".............aXXaaXXXXXa",
                ".............aXXaXXXXXXa",
                ".............aXXXaXXXXXa",
                ".............aXXXXXXXXXa",
                ".............aaaaaaaaaaa"
};

// QPixmap* KApp::getPixmapCursor(Qt::CursorShape cshape)
// {
// #if defined(Q_WS_X11) || defined(Q_WS_WIN)
//     if (!move_cursor) {
//         move_cursor = new QPixmap((const char **)move_xpm);
//         copy_cursor = new QPixmap((const char **)copy_xpm);
//         link_cursor = new QPixmap((const char **)link_xpm);
// #ifdef Q_WS_WIN
//         ignore_cursor = new QPixmap((const char **)ignore_xpm);
// #endif
//     }
// 
//     switch (cshape) {
//     case Qt::DragMoveCursor:
//         return move_cursor;
//     case Qt::DragCopyCursor:
//         return copy_cursor;
//     case Qt::DragLinkCursor:
//         return link_cursor;
// #ifdef Q_WS_WIN
//     case Qt::ForbiddenCursor:
//         return ignore_cursor;
// #endif
//     default:
//         break;
//     }
// #else
//     Q_UNUSED(cshape);
// #endif
//     return 0;
}

//Returns the current platform used by keyBindings
unsigned int KApp::currentPlatform(){
    uint platform = KB_Win;
#ifdef Q_WS_MAC
    platform = KB_Mac;
#elif defined Q_WS_X11
    platform = KB_X11;
    if (X11->desktopEnvironment == DE_KDE)
        platform |= KB_KDE;
    if (X11->desktopEnvironment == DE_GNOME)
        platform |= KB_Gnome;
    if (X11->desktopEnvironment == DE_CDE)
        platform |= KB_CDE;
#elif defined(Q_OS_SYMBIAN)
    platform = KB_S60;
#endif
    return platform;
}

#endif // QT_NO_CLIPBOARD