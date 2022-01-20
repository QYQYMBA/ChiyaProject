#ifndef WINAPIADAPTER_H
#define WINAPIADAPTER_H

#include <QString>

#include <windows.h>
#include <sstream>
#include <iomanip>

class WinApiAdapter
{
public:
    static QString GetWindowName(HWND hwnd);
    static QString GetWindowClass(HWND hwnd);
    static QString GetWindowPath(HWND hwnd);
    static QString GetWindowExeName(HWND hwnd);

    static std::vector<HKL> getLayoutsList();
    static std::string hklToStr(HKL hkl);

    static void SetKeyboardLayout(HKL layout);

    template< typename T >
    static std::string decToHex(T i);

    static INPUT MakeKeyInput(int vkCode, bool down);

    static HKL GetLayoutByHwnd(HWND hwnd);
    static HKL GetLayout();

    static void SendKeyPress(int vkCode, bool shift = false, bool ctrl = false, bool alt = false);
private:
    WinApiAdapter();
};

template< typename T >
std::string WinApiAdapter::decToHex(T i)
{
    std::stringstream stream;
    stream << "0x"
        << std::setfill('0') << std::setw(sizeof(T) * 2)
        << std::hex << i;
    return stream.str();
}

#endif // WINAPIADAPTER_H
