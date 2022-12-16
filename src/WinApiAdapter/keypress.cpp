#include "keypress.h"

#include <QDebug>

KeyPress::KeyPress()
{

}

KeyPress::KeyPress(RAWKEYBOARD key, bool shift, bool caps, bool ctrl, bool alt, HKL keyboardLayout, bool getKeyboardState)
{
    this->vkCode = key.VKey;
    this->scanCode = key.MakeCode;

    this->shift = shift;
    this->caps = caps;
    this->ctrl = ctrl;

    this->alt = alt;

    this->keyboardLayout = keyboardLayout;

    if(getKeyboardState)
    {
        GetKeyboardState(keyboardState);
    }
    else
    {

    }
}

KeyPress::KeyPress(int vkCode, int scanCode, bool shift, bool caps, bool ctrl, bool alt, HKL keyboardLayout, bool getKeyboardState)
{
    this->vkCode = vkCode;
    this->scanCode = scanCode;

    this->shift = shift;
    this->caps = caps;
    this->ctrl = ctrl;

    this->alt = alt;

    this->keyboardLayout = keyboardLayout;

    if(getKeyboardState)
    {
        GetKeyboardState(keyboardState);
    }
    else
    {

    }
}

KeyPress::KeyPress(PKBDLLHOOKSTRUCT key, bool shift, bool caps, bool ctrl, bool alt, HKL keyboardLayout, bool getKeyboardState)
{
    if (key != nullptr) {
        this->vkCode = key->vkCode;
        this->scanCode = key->scanCode;
    }
    else
    {
        this->vkCode = 0;
        this->scanCode = 0;
    }
    this->shift = shift;
    this->caps = caps;
    this->ctrl = ctrl;

    this->alt = alt;

    this->keyboardLayout = keyboardLayout;

    if(getKeyboardState)
    {
        GetKeyboardState(keyboardState);
    }
    else
    {

    }
}

KeyPress::KeyPress(LPARAM lParam, bool shift, bool caps, bool ctrl, bool alt, HKL keyboardLayout, bool getKeyboardState)
{
    PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT) lParam;

    if (key != nullptr) {
        this->vkCode = key->vkCode;
        this->scanCode = key->scanCode;
    }
    else
    {
        this->vkCode = 0;
        this->scanCode = 0;
    }
    this->shift = shift;
    this->caps = caps;
    this->ctrl = ctrl;

    this->alt = alt;

    this->keyboardLayout = keyboardLayout;

    if(getKeyboardState)
    {
        GetKeyboardState(keyboardState);
    }
    else
    {

    }
}

QChar KeyPress::toChar() {
    WCHAR uc[5] = {};

    GetKeyboardState(keyboardState);

    int res = ToUnicodeEx(vkCode, scanCode, keyboardState, uc, 4, 0, keyboardLayout);
    if(res >= 1)
    {
        QChar k = QString::fromWCharArray(uc)[0];
        return k;
    }

    return ' ';
}

QChar KeyPress::toChar(HKL layout) {
    WCHAR uc[5] = {};

    memset(keyboardState, 0, sizeof(keyboardState));

    int res = ToUnicodeEx(vkCode, MapVirtualKey(vkCode, 0), keyboardState, uc, 4, 0, layout);
    if(res >= 1)
    {
        QChar k = QString::fromWCharArray(uc)[0];
        QString s = k;
        return k;
    }

    return ' ';
}

bool KeyPress::isPrintable() {
    int vkCode = this->vkCode;
    if (vkCode == VK_SHIFT) { return true; }
    if (vkCode == VK_MENU || vkCode == VK_LWIN  || vkCode == VK_PAUSE) { return false; }
    if (vkCode >= 48 && vkCode <= 90) { return true; }
    if (vkCode >= 186 && vkCode <= 226) { return true; }
    if (vkCode >= 106 && vkCode <= 111) { return true; }
    if (vkCode >= VK_NUMPAD0 && vkCode <= VK_NUMPAD9) { return true; }
    if (vkCode == VK_DECIMAL) { return true; }
    if (vkCode == VK_SPACE) { return false; }
    return false;
}

int KeyPress::getVkCode()
{
    return vkCode;
}

HKL KeyPress::getLayout()
{
    return keyboardLayout;
}

bool KeyPress::isShiftPressed()
{
    return shift;
}

bool KeyPress::isCtrlPressed()
{
    return ctrl;
}

bool KeyPress::isAltPressed()
{
    return alt;
}

KeyPress KeyPress::CharToKey(QChar ch, const HKL layout)
{
    wchar_t wch = ch.unicode();

    SHORT keyNumber = VkKeyScanExW(wch, layout);

    if (keyNumber == -1)
    {
        return 0;
    }
    PKBDLLHOOKSTRUCT key1 = new KBDLLHOOKSTRUCT;
    key1->vkCode = LOBYTE(keyNumber);
    bool shift = HIBYTE(keyNumber) & 1;
    KeyPress key(key1, shift);
    key.scanCode = MapVirtualKey(key1->vkCode, MAPVK_VK_TO_VSC);
    return key;
}
