#include "KApp.h"
#include <windows.h>

#define SPI_GETMOUSECLICKLOCKTIME           0x2008
#define SPI_SETMOUSECLICKLOCKTIME           0x2009
#define SPI_GETFONTSMOOTHINGTYPE            0x200A
#define SPI_SETFONTSMOOTHINGTYPE            0x200B

/* constants for SPI_GETFONTSMOOTHINGTYPE and SPI_SETFONTSMOOTHINGTYPE: */
#define FE_FONTSMOOTHINGSTANDARD            0x0001
#define FE_FONTSMOOTHINGCLEARTYPE           0x0002
#define FE_FONTSMOOTHINGDOCKING             0x8000

void KApp::beep()
{
    ::MessageBeep(MB_OK);
}

#ifndef QT_NO_WHEELEVENT
void KApp::setWheelScrollLines(int n)
{
#ifdef SPI_SETWHEELSCROLLLINES
    if (n < 0)
        n = 0;
    SystemParametersInfo(SPI_SETWHEELSCROLLLINES, (uint)n, 0, 0);
#else
    QApplicationPrivate::wheel_scroll_lines = n;
#endif
}

int KApp::wheelScrollLines()
{
#ifdef SPI_GETWHEELSCROLLLINES
    uint i = 3;
    SystemParametersInfo(SPI_GETWHEELSCROLLLINES, sizeof(uint), &i, 0);
    if (i > INT_MAX)
        i = INT_MAX;
    return i;
#else
    return QApplicationPrivate::wheel_scroll_lines;
#endif
}

static HDC displayDC = 0;                // display device context
HDC qt_win_display_dc()                        // get display DC
{
    //Q_ASSERT(qApp && qApp->thread() == QThread::currentThread());
    if (!displayDC)
        displayDC = GetDC(0);
    return displayDC;
}

void qt_win_read_cleartype_settings()
{
    UINT result = 0;
#ifdef Q_OS_WINCE
    if (SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &result, 0))
        qt_cleartype_enabled = result;
#else
    if (SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &result, 0))
        qt_cleartype_enabled = (result == FE_FONTSMOOTHINGCLEARTYPE);
#endif

    int winSmooth;
    if (SystemParametersInfo(0x200C /* SPI_GETFONTSMOOTHINGCONTRAST */, 0, &winSmooth, 0)) {
        qt_fontsmoothing_gamma = winSmooth / double(1000.0);
    } else {
        qt_fontsmoothing_gamma = 1.0;
    }

    // Safeguard ourselves against corrupt registry values...
    if (qt_fontsmoothing_gamma > 5 || qt_fontsmoothing_gamma < 1)
        qt_fontsmoothing_gamma = double(1.4);
}


#endif //QT_NO_WHEELEVENT
