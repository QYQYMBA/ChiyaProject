#ifndef KEY_H
#define KEY_H

#include <windows.h>

class Key {
    public:
        Key(PKBDLLHOOKSTRUCT key, bool shift = false, bool caps = false, bool ctrl = false);
        Key();
        int vkCode, scanCode;
        bool shift, caps, ctrl, alt;
        char toChar();
        char toChar(HKL layout);
        bool isPrintable();
        static Key CharToKey(char ch, const HKL layout);
};

#endif // KEY_H
