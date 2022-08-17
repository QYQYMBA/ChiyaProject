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
    if (_keyboardFocus != NULL)
    {
        _automation->RemovePropertyChangedEventHandler(_keyboardFocus, (IUIAutomationPropertyChangedEventHandler*)_pceh);
        _keyboardFocus->Release();
    }
    _elementChanged = true;

    _keyboardFocus = sender;
    if(_keyboardFocus == NULL)
        return S_FALSE;
    _keyboardFocus->AddRef();

    CComSafeArray<PROPERTYID> sf(1);
    sf[0] = UIA_ValueValuePropertyId;
    _automation->AddPropertyChangedEventHandler(_keyboardFocus, TreeScope_Element, NULL, (IUIAutomationPropertyChangedEventHandler*)_pceh, sf);
    //_automation->AddPropertyChangedEventHandler(_keyboardFocus, TreeScope_Subtree, NULL, (IUIAutomationPropertyChangedEventHandler*)_pceh, sf);

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
