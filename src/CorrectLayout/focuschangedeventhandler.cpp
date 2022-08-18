#include "QtDebug"

#include <atlbase.h>
#include <atlsafe.h>
#include "focuschangedeventhandler.h"

FocusChangedEventHandler::FocusChangedEventHandler(IUIAutomation* automation, PropertyChangedEventHandler* pceh)
{
    _pceh = pceh;
    _automation = automation;
    _keyboardFocus = NULL;
    _elementChanged = true;
}

IUIAutomationElement* FocusChangedEventHandler::findElement(IUIAutomationElement* element)
{
    if(element == NULL)
    {
        return NULL;
    }

    IUIAutomationCondition* hasKeyboardFocusCondition;
    IUIAutomationCondition* isTextPatternAvailable;
    IUIAutomationCondition* textHasKeyboardFocus;
    IUIAutomationCondition* notPassword;
    IUIAutomationCondition* searchCondition;
    IUIAutomationElement* keyboardFocus = NULL;
    VARIANT trueVar;
    trueVar.vt = VT_BOOL;
    trueVar.boolVal = VARIANT_TRUE;
    VARIANT falseVar;
    falseVar.vt = VT_BOOL;
    falseVar.boolVal = VARIANT_FALSE;
    HRESULT hr = _automation->CreatePropertyCondition(UIA_HasKeyboardFocusPropertyId, trueVar, &hasKeyboardFocusCondition);
    if (FAILED(hr))
    {
        qDebug() << "Failed to CreatePropertyCondition";
        return NULL;
    }
    hr = _automation->CreatePropertyCondition(UIA_IsTextPatternAvailablePropertyId, trueVar, &isTextPatternAvailable);
    if (FAILED(hr))
    {
        qDebug() << "Failed to CreatePropertyCondition";
        return NULL;
    }
    hr = _automation->CreatePropertyCondition(UIA_IsPasswordPropertyId, falseVar, &notPassword);
    if (FAILED(hr))
    {
        qDebug() << "Failed to CreatePropertyCondition";
        return NULL;
    }

    _automation->CreateAndCondition(hasKeyboardFocusCondition, isTextPatternAvailable, &textHasKeyboardFocus);
    if (FAILED(hr))
    {
        qDebug() << "Failed to CreateAndCondition";
        return NULL;
    }

    _automation->CreateAndCondition(textHasKeyboardFocus, notPassword, &searchCondition);
    if (FAILED(hr))
    {
        qDebug() << "Failed to CreateAndCondition";
        return NULL;
    }
    else
    {
        hr = element->FindFirst(TreeScope_Subtree, searchCondition, &keyboardFocus);
        searchCondition->Release();
        if (FAILED(hr))
        {
            qDebug() << "FindFirst failed";
            return NULL;
        }
        else if (keyboardFocus == NULL)
        {
            keyboardFocus = element;
        }
        hr = _automation->CreatePropertyCondition(UIA_IsPasswordPropertyId, trueVar, &notPassword);
        if (FAILED(hr))
        {
            qDebug() << "Failed to CreatePropertyCondition";
            return NULL;
        }
    }
    return keyboardFocus;
}

bool FocusChangedEventHandler::elementChanged(bool reset)
{
    bool o = _elementChanged;
    if (reset)
        _elementChanged = false;
    return o;
}

HRESULT FocusChangedEventHandler::changeValue(QString newValue, int position)
{
    if(_elementChanged)
        return S_FALSE;

    BSTR bstrString;
    BOOL f;
    int moved;
    std::wstring w = newValue.toStdWString();
    bstrString = SysAllocString(w.c_str());
    avp->SetValue(bstrString);
    atp->GetCaretRange(&f, &atr);
    if (atr != NULL)
    {
        atr->Move(TextUnit_Character, -newValue.size(), &moved);
        atr->Move(TextUnit_Character, position + 1, &moved);
        atr->Select();
    }
    _keyboardFocus->SetFocus();
    return S_OK;
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
        hr = _automation->RemovePropertyChangedEventHandler(_keyboardFocus, (IUIAutomationPropertyChangedEventHandler*)_pceh);
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

    _keyboardFocus = findElement(sender);
    if(_keyboardFocus == NULL)
        return S_FALSE;
    hr = _keyboardFocus->AddRef();
    if (FAILED(hr))
    {
        qDebug() << "Can't add reference";
        return hr;
    }

    hr = _keyboardFocus->GetCurrentPattern(UIA_LegacyIAccessiblePatternId, (IUnknown**)&avp);
    if (FAILED(hr))
    {
        qDebug() << "Can't get current LegacyIAccessiblePattern";
        return hr;
    }
    hr = _keyboardFocus->GetCurrentPattern(UIA_TextPattern2Id, (IUnknown**)&atp);
    if (FAILED(hr))
    {
        qDebug() << "Can't get current TextPattern2";
        return hr;
    }

    CComSafeArray<PROPERTYID> sf(1);
    sf[0] = UIA_ValueValuePropertyId;
    hr = _automation->AddPropertyChangedEventHandler(_keyboardFocus, TreeScope_Subtree, NULL, (IUIAutomationPropertyChangedEventHandler*)_pceh, sf);
    if (FAILED(hr))
    {
        qDebug() << "Can't add property changed event handler";
        return hr;
    }
    _pceh->updateText(_keyboardFocus);

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
