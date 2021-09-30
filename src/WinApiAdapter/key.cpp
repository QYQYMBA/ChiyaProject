#include "key.h"

#include <string>
#include <QDebug>

#include "winapiadapter.h"

Key::Key(PKBDLLHOOKSTRUCT key, bool shift, bool caps, bool ctrl, bool alt) {
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
}

Key::Key(RAWKEYBOARD key, bool shift, bool caps, bool ctrl, bool alt) {
    this->vkCode = key.VKey;
    this->scanCode = key.MakeCode;

    this->shift = shift;
    this->caps = caps;
    this->ctrl = ctrl;

    this->alt = alt;
}

Key::Key()
{

}

char Key::toChar() {
    BYTE lpKeyState[256];
    bool success = GetKeyboardState(lpKeyState);
    lpKeyState[VK_SHIFT] = this->shift ? 129 : 0;
    lpKeyState[VK_LSHIFT] = this->shift;
    lpKeyState[VK_CAPITAL] = this->caps;
    WORD k;
    ToAsciiEx(this->vkCode, this->scanCode, lpKeyState, &k, NULL, GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL)));
    char c = k - 52480;
    return c;
}

char Key::toChar(HKL layout) {
    BYTE lpKeyState[256];
    bool success = GetKeyboardState(lpKeyState);
    lpKeyState[VK_SHIFT] = this->shift ? 129 : 0;
    lpKeyState[VK_LSHIFT] = this->shift;
    lpKeyState[VK_CAPITAL] = this->caps;
    WORD k;
    ToAsciiEx(this->vkCode, this->scanCode, lpKeyState, &k, NULL, layout);
    char c = k > 50000 ? k - 52480 : k;
    return c;
}

bool Key::isPrintable() {
    int vkCode = this->vkCode;
    if (vkCode == VK_MENU || vkCode == VK_SHIFT || vkCode == VK_LWIN  || vkCode == VK_PAUSE) { return false; }
    if (vkCode >= 48 && vkCode <= 90) { return true; }
    if (vkCode >= 186 && vkCode <= 226) { return true; }
    if (vkCode >= 106 && vkCode <= 111) { return true; }
    if (vkCode >= VK_NUMPAD0 && vkCode <= VK_NUMPAD9) { return true; }
    if (vkCode == VK_DECIMAL) { return true; }
    if (vkCode == VK_SPACE) { return false; }
    return false;
}

Key Key::CharToKey(char ch, const HKL layout)
{
    std::string s;
    s[0] = ch;

    SHORT keyNumber = VkKeyScanEx(ch, layout);

    if (keyNumber == -1)
    {
        return 0;
    }
    PKBDLLHOOKSTRUCT key1 = new KBDLLHOOKSTRUCT;
    key1->vkCode = LOBYTE(keyNumber);
    bool shift = HIBYTE(keyNumber) & 1;
    Key key(key1, shift);
    key.scanCode = 0;
    return Key(key);
}
