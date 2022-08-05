#include "llkeyhandler.h"

#include "winapiadapter.h"

LlKeyHandler::LlKeyHandler()
    :_userKeyPresses()
{
    _state = false;
    _result = false;
}

bool LlKeyHandler::handleLlKey()
{
    PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT) lParam;
    if(key->flags & LLKHF_INJECTED)
    {
        if(key->vkCode == VK_PAUSE && _state == true)
        {
            _state = false;
            _switcherWork = true;
            while(!_userKeyPresses.empty())
            {
                WinApiAdapter::SendKeyPress(_userKeyPresses.dequeue());
            }
        }
        return false;
    }
    else
    {
        if(_state)
        {
            bool ctrl = GetAsyncKeyState(VK_CONTROL);
            bool shift = GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT);
            bool alt = GetAsyncKeyState(VK_MENU);

            KeyPress keyPress(key, shift, true, ctrl, alt);
            _userKeyPresses.enqueue(keyPress);
            return true;
        }
        else
        {
            while(!_userKeyPresses.empty())
            {
                WinApiAdapter::SendKeyPress(_userKeyPresses.dequeue());
            }
        }
    }
    return false;
}

bool LlKeyHandler::getReslut()
{
    return _result;
}

void LlKeyHandler::passArguments(int nCode, WPARAM wParam, LPARAM lParam)
{
    this->nCode = nCode;
    this->wParam = wParam;
    this->lParam = lParam;
}

void LlKeyHandler::blockInput()
{
    _state = true;
}

bool LlKeyHandler::switcherWork()
{
    if(_switcherWork)
    {
        _switcherWork = false;
        return true;
    }

    return false;
}

void LlKeyHandler::run()
{
    _result = false;
    _result = handleLlKey();
}
