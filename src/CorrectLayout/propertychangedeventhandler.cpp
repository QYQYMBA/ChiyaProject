#include "propertychangedeventhandler.h"
#include "correctlayout.h"

#include "QtDebug"

#include <atlbase.h>
#include <atlsafe.h>
#include <tuple>
#include <propvarutil.h>

PropertyChangedEventHandler::PropertyChangedEventHandler(IUIAutomation *automation, void* cl)
{
    _automation = automation;
    mutex.lock();
    _cl = cl;
}

IUIAutomationElement* PropertyChangedEventHandler::findElement(IUIAutomationElement* element)
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

QString PropertyChangedEventHandler::getElementText(IUIAutomationElement* element)
{
    if(element == NULL)
    {
        return "";
    }

    VARIANT text;
    HRESULT hr = element->GetCurrentPropertyValue(UIA_ValueValuePropertyId ,&text);
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

HRESULT PropertyChangedEventHandler::HandlePropertyChangedEvent(IUIAutomationElement *sender, PROPERTYID propertyId, VARIANT newValue)
{
    ((CorrectLayout*)_cl)->handleValueChange(getElementText(findElement(sender)));
    return S_OK;
}

HRESULT PropertyChangedEventHandler::QueryInterface(const IID &riid, LPVOID *ppvObj)
{
    // Always set out parameter to NULL, validating it first.
        if (!ppvObj)
            return E_INVALIDARG;
        *ppvObj = NULL;
        if (riid == IID_IUnknown || riid == IID_IUIAutomationPropertyChangedEventHandler)
        {
            // Increment the reference count and return the pointer.
            *ppvObj = (LPVOID)this;
            AddRef();
            return NOERROR;
        }
        return E_NOINTERFACE;
}


ULONG PropertyChangedEventHandler::AddRef()
{
    InterlockedIncrement(&_cRef);
    return _cRef;
}

ULONG PropertyChangedEventHandler::Release()
{
    // Decrement the object's internal counter.
        ULONG ulRefCount = InterlockedDecrement(&_cRef);
        if (0 == _cRef)
        {
            delete this;
        }
        return ulRefCount;
}
