#include "QtDebug"

#include <atlbase.h>
#include <atlsafe.h>
#include "focuschangedeventhandler.h"

QString FocusChangedEventHandler::getElementText(IUIAutomationElement* element)
{
    if(element == NULL)
    {
        return "";
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
        return "";
    }
    hr = _automation->CreatePropertyCondition(UIA_IsTextPatternAvailablePropertyId, trueVar, &isTextPatternAvailable);
    if (FAILED(hr))
    {
        qDebug() << "Failed to CreatePropertyCondition";
        return "";
    }
    hr = _automation->CreatePropertyCondition(UIA_IsPasswordPropertyId, falseVar, &notPassword);
    if (FAILED(hr))
    {
        qDebug() << "Failed to CreatePropertyCondition";
        return "";
    }

    _automation->CreateAndCondition(hasKeyboardFocusCondition, isTextPatternAvailable, &textHasKeyboardFocus);
    if (FAILED(hr))
    {
        qDebug() << "Failed to CreateAndCondition";
        return "";
    }

    _automation->CreateAndCondition(textHasKeyboardFocus, notPassword, &searchCondition);
    if (FAILED(hr))
    {
        qDebug() << "Failed to CreateAndCondition";
        return "";
    }
    else
    {
        hr = element->FindFirst(TreeScope_Subtree, searchCondition, &keyboardFocus);
        searchCondition->Release();
        if (FAILED(hr))
        {
            qDebug() << "FindFirst failed";
            return "";
        }
        else if (keyboardFocus == NULL)
        {
            keyboardFocus = element;
        }
        hr = _automation->CreatePropertyCondition(UIA_IsPasswordPropertyId, trueVar, &notPassword);
        if (FAILED(hr))
        {
            qDebug() << "Failed to CreatePropertyCondition";
            return "";
        }
    }
    VARIANT text;
    hr = keyboardFocus->GetCurrentPropertyValue(UIA_ValueValuePropertyId ,&text);
    if(keyboardFocus != element)
        keyboardFocus->Release();
    if (FAILED(hr))
    {
        qDebug() << "Failed to get element value.";
        return "";
    }
    if(SUCCEEDED(hr))
    {
        wchar_t* t = text.bstrVal;
        QString s = QString::fromWCharArray(t);
        return s;
    }
    return "";
}

FocusChangedEventHandler::FocusChangedEventHandler(IUIAutomation* automation)
{
    _sender = NULL;
    _automation = automation;
}

QString FocusChangedEventHandler::getElementText()
{
    _elementChanged = false;
    return getElementText(_sender);
}

bool FocusChangedEventHandler::elementChanged()
{
    _elementChanged = false;
    return _elementChanged;
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
    if(_sender != NULL)
        _sender->Release();
    _elementChanged = true;
    _sender = sender;
    CComSafeArray<PROPERTYID> sf(1);
    sf[0] = UIA_ValueValuePropertyId;
    _pceh = new PropertyChangedEventHandler(_automation);
    HRESULT hr = _automation->AddPropertyChangedEventHandler(_sender, TreeScope_Subtree, NULL, (IUIAutomationPropertyChangedEventHandler*)_pceh, sf);
    hr = _automation->AddPropertyChangedEventHandler(_sender, TreeScope_Element, NULL, (IUIAutomationPropertyChangedEventHandler*)_pceh, sf);
    _sender->AddRef();

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
