#ifndef LLKEYHANDLER_H
#define LLKEYHANDLER_H

#include <QThread>
#include <QQueue>
#include "windows.h"
#include "keypress.h"

class LlKeyHandler : public QThread
{
public:
    LlKeyHandler();
    bool handleLlKey();
    bool getReslut();
    void passArguments(int nCode, WPARAM wParam, LPARAM lParam);

    int nCode;
    WPARAM wParam;
    LPARAM lParam;
private:
    bool result;
    QQueue<KeyPress> _userKeyPresses;
    bool _state;
protected:
    void run();
};

#endif // LLKEYHANDLER_H
