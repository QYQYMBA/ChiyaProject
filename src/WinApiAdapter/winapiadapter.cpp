#include "winapiadapter.h"

QString WinApiAdapter::GetWindowName(HWND hwnd)
{
    WCHAR buffer[256];
    GetWindowText(hwnd, buffer, 20);
    char ch[260];
    char DefChar = ' ';
    WideCharToMultiByte(CP_ACP, 0, buffer, -1, ch, 260, &DefChar, NULL);
    QString className(ch);

    return className;
}

QString WinApiAdapter::GetWindowClass(HWND hwnd)
{
    WCHAR buffer[256];
    GetClassNameW(hwnd, buffer, 20);
    char ch[260];
    char DefChar = ' ';
    WideCharToMultiByte(CP_ACP, 0, buffer, -1, ch, 260, &DefChar, NULL);
    QString className(ch);

    return className;
}

WinApiAdapter::WinApiAdapter()
{
}
