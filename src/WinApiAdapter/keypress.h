#ifndef KEYPRESS_H
#define KEYPRESS_H

#include "Windows.h"

#include <QString>

class KeyPress
{
public:
    KeyPress(RAWKEYBOARD key, bool shift = false, bool caps = false, bool ctrl = false, bool alt = false, HKL keyboardLayout = 0, bool getKeyboardState = true);
    KeyPress(int vkCode, int scanCode = 0, bool shift = false, bool caps = false, bool ctrl = false, bool alt = false, HKL keyboardLayout = 0, bool getKeyboardState = true);
    KeyPress(PKBDLLHOOKSTRUCT key, bool shift = false, bool caps = false, bool ctrl = false, bool alt = false, HKL keyboardLayout = 0, bool getKeyboardState = true);
    KeyPress(LPARAM lParam, bool shift = false, bool caps = false, bool ctrl = false, bool alt = false, HKL keyboardLayout = 0, bool getKeyboardState = true);
    KeyPress();
    QChar toChar();
    QChar toChar(HKL layout);
    bool isPrintable();
    int getVkCode();
    HKL getLayout();
    bool isShiftPressed();
    bool isCtrlPressed();
    bool isAltPressed();

    static KeyPress CharToKey(QChar ch, const HKL layout);
private:
    int vkCode, scanCode;
    bool shift, caps, ctrl, alt;
    BYTE keyboardState[256];
    HKL keyboardLayout;
};

#endif // KEYPRESS_H
