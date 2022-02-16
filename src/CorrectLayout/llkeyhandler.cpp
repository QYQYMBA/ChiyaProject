#include "llkeyhandler.h"

#include "winapiadapter.h"

LlKeyHandler::LlKeyHandler()
    :_userKeyPresses()
{
    _state = false;
    result = false;
}

bool LlKeyHandler::handleLlKey()
{
    PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT) lParam;
    if(key->flags & LLKHF_INJECTED)
    {
        if(key->vkCode == VK_PAUSE)
        {
            _state = false;
            while(!_userKeyPresses.empty())
            {
                WinApiAdapter::SendKeyPress(_userKeyPresses.dequeue());
            }
        }
        if(key->vkCode == VK_SCROLL)
        {
            _state = true;
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
    return result;
}

void LlKeyHandler::passArguments(int nCode, WPARAM wParam, LPARAM lParam)
{
    this->nCode = nCode;
    this->wParam = wParam;
    this->lParam = lParam;
}

void LlKeyHandler::run()
{
    result = false;
    result = handleLlKey();
}
