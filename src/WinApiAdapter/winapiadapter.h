#ifndef WINAPIADAPTER_H
#define WINAPIADAPTER_H

#include <QString>
#include <QVector>

#include <windows.h>
#include <sstream>
#include <iomanip>

#include "keypress.h"

class WinApiAdapter
{
public:
    static QString GetWindowName(HWND hwnd);
    static QString GetWindowClass(HWND hwnd);
    static QString GetWindowPath(HWND hwnd);
    static QString GetWindowExeName(HWND hwnd);

    static std::vector<HKL> getLayoutsList();
    static QString hklToStr(HKL hkl);
    static QString vkToString(uint vkCode);

    static void SetKeyboardLayout(HKL layout);

    template< typename T >
    static QString decToHex(T i);

    static INPUT MakeKeyInput(int vkCode, bool down);

    static HKL GetLayoutByHwnd(HWND hwnd);
    static HKL GetLayout();

    static void ReplaceUnicodeString(QString s);
    static void SendUnicodeString(QString s);
    static void SendUnicodeChar(QChar c);
    static void SendKeyPress(int vkCode, bool shift = false, bool ctrl = false, bool alt = false);
    static void SendKeyPress(KeyPress kp);
    static void SendKeyPresses(QVector<KeyPress> keyPresses);
private:
    WinApiAdapter();
};

template< typename T >
QString WinApiAdapter::decToHex(T i)
{
    std::stringstream stream;
    stream << "0x"
        << std::setfill('0') << std::setw(sizeof(T) * 2)
        << std::hex << i;
    std::string ss = stream.str();
    return QString::fromStdString(ss);
}

#endif // WINAPIADAPTER_H
