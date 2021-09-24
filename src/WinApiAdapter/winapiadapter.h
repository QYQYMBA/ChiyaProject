#ifndef WINAPIADAPTER_H
#define WINAPIADAPTER_H

#include <QString>

#include <windows.h>

class WinApiAdapter
{
public:
    static QString GetWindowName(HWND hwnd);
    static QString GetWindowClass(HWND hwnd);
private:
    WinApiAdapter();
};

#endif // WINAPIADAPTER_H
