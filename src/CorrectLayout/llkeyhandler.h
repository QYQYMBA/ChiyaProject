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

    void blockInput();
    bool switcherWork();

    int nCode;
    WPARAM wParam;
    LPARAM lParam;
private:
    bool _result;
    bool _state;
    bool _switcherWork;
    QQueue<KeyPress> _userKeyPresses;

protected:
    void run();
};

#endif // LLKEYHANDLER_H
