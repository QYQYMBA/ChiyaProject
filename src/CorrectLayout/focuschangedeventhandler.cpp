#include "QtDebug"

#include <atlbase.h>
#include <atlsafe.h>
#include "focuschangedeventhandler.h"

FocusChangedEventHandler::FocusChangedEventHandler(IUIAutomation* automation, AutomationEventHandle* pceh, LayoutController* layoutController, bool passwordCapsLock, HKL passwordLayout)
{
    _aeh = pceh;
    _automation = automation;
    _layoutController = layoutController;
    _passwordCapsLock = passwordCapsLock;
    _passwordLayout = passwordLayout;

    _keyboardFocus = NULL;
    _elementChanged = true;
}

bool FocusChangedEventHandler::elementChanged(bool reset)
{
    bool o = _elementChanged;
    if (reset)
        _elementChanged = false;
    return o;
}

HRESULT FocusChangedEventHandler::QueryInterface(const IID &riid, LPVOID *ppvObj)
{
    // Always set out parameter to NULL, validating it first.
        if (!ppvObj)
            return E_INVALIDARG;
        *ppvObj = NULL;
        if (riid == IID_IUnknown || riid == IID_IUIAutomationFocusChangedEventHandler)
        {
            // Increment the reference count and return the pointer.
            *ppvObj = (LPVOID)this;
            AddRef();
            return NOERROR;
        }
        return E_NOINTERFACE;
}

HRESULT FocusChangedEventHandler::activateTextChangedHandler(bool state)
{
    HRESULT hr;
    if(state)
    {
        hr = _automation->AddAutomationEventHandler(UIA_Text_TextChangedEventId, _keyboardFocus, TreeScope_Element, NULL, (IUIAutomationEventHandler*)_aeh);
        if (FAILED(hr))
        {
            qDebug() << "Can't add property automation event handler";
            return hr;
        }
    }
    else
    {
        hr = _automation->RemoveAutomationEventHandler(UIA_Text_TextChangedEventId, _keyboardFocus, (IUIAutomationEventHandler*)_aeh);

        if (FAILED(hr))
        {
            qDebug() << "Can't remove event handler";
            return hr;
        }
    }
}

HRESULT FocusChangedEventHandler::HandleFocusChangedEvent(IUIAutomationElement *sender)
{
    HWND hwnd = GetForegroundWindow();
    if (hwnd == NULL) return false;

    DWORD foregroundPid;
    if (GetWindowThreadProcessId(hwnd, &foregroundPid) == 0) return false;

    if (foregroundPid == GetCurrentProcessId())
        return S_FALSE;

    HRESULT hr;
    if (_keyboardFocus != NULL)
    {
        hr = _automation->RemoveAutomationEventHandler(UIA_Text_TextChangedEventId, _keyboardFocus, (IUIAutomationEventHandler*)_aeh);

        if (FAILED(hr))
        {
            qDebug() << "Can't remove event handler";
            return hr;
        }
        hr =_keyboardFocus->Release();
        if (FAILED(hr))
        {
            qDebug() << "Can't release keyboardFocus";
            return hr;
        }
    }
    _elementChanged = true;

    _keyboardFocus = sender;
    if(_keyboardFocus == NULL)
    {
        return S_FALSE;
    }
    else
    {
        if(_passwordCapsLock || _passwordLayout != 0x0)
        {
            VARIANT isPassword;
            _keyboardFocus->GetCurrentPropertyValue(UIA_IsPasswordPropertyId, &isPassword);
            if(isPassword.boolVal)
            {
                if(_passwordCapsLock)
                {

                }
                if (_passwordLayout != 0x0)
                {
                    _layoutController->switchLayout(_passwordLayout);
                }
            }
        }
    }
    hr = _keyboardFocus->AddRef();
    if (FAILED(hr))
    {
        qDebug() << "Can't add reference";
        return hr;
    }

    hr = _automation->AddAutomationEventHandler(UIA_Text_TextChangedEventId, _keyboardFocus, TreeScope_Element, NULL, (IUIAutomationEventHandler*)_aeh);
    if (FAILED(hr))
    {
        qDebug() << "Can't add property automation event handler";
        return hr;
    }
    _aeh->updateText(_keyboardFocus);

    return S_OK;
}

ULONG FocusChangedEventHandler::AddRef()
{
    InterlockedIncrement(&_cRef);
    return _cRef;
}

ULONG FocusChangedEventHandler::Release()
{
    // Decrement the object's internal counter.
        ULONG ulRefCount = InterlockedDecrement(&_cRef);
        if (0 == _cRef)
        {
            delete this;
        }
        return ulRefCount;
}
