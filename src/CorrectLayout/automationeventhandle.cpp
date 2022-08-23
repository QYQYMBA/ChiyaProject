#include "automationeventhandle.h"
#include "correctlayout.h"

#include "QtDebug"

#include <atlbase.h>
#include <atlsafe.h>
#include <tuple>
#include <propvarutil.h>

AutomationEventHandle::AutomationEventHandle(IUIAutomation *automation, void* cl)
{
    _automation = automation;
    _cl = cl;
    _lastElement = NULL;
}

HRESULT AutomationEventHandle::updateText(IUIAutomationElement *element)
{
    return HandleAutomationEvent(element, NULL);
}

IUIAutomationElement* AutomationEventHandle::findElement(IUIAutomationElement* element)
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

QString AutomationEventHandle::getElementText(IUIAutomationElement* element)
{
    if(element == NULL)
    {
        return "";
    }

    IUIAutomationTextPattern* atp;
    IUIAutomationTextRange* atr;
    BSTR bstr;
    QString str = "";
    HRESULT hr = element->GetCurrentPattern(UIA_TextPatternId, (IUnknown**)&atp);
    if (FAILED(hr) || atp == NULL)
    {
        qDebug() << "Failed to get text pattern.";
        return "";
    }
    atp->get_DocumentRange(&atr);
    if (FAILED(hr))
    {
        qDebug() << "Failed to get document range.";
        return "";
    }
    atr->GetText(300, &bstr);
    if (FAILED(hr))
    {
        qDebug() << "Failed to get element text.";
        return "";
    }
    str = QString::fromStdWString(bstr);
    return str;
}

HRESULT AutomationEventHandle::HandleAutomationEvent(IUIAutomationElement *sender, EVENTID eventId)
{
    QString currentText = getElementText(sender);
    if (_oldText == currentText)
        return S_FALSE;
    else
        _oldText = currentText;
    ((CorrectLayout*)_cl)->handleValueChange(currentText);

    if (_lastElement != NULL)
    {
        _lastElement->Release();
    }
    _lastElement = sender;
    if(_lastElement == NULL)
        return S_FALSE;
    HRESULT hr = _lastElement->AddRef();
    if (FAILED(hr))
    {
        qDebug() << "Can't add reference";
        return hr;
    }

    return S_OK;
}

HRESULT AutomationEventHandle::QueryInterface(const IID &riid, LPVOID *ppvObj)
{
    // Always set out parameter to NULL, validating it first.
        if (!ppvObj)
            return E_INVALIDARG;
        *ppvObj = NULL;
        if (riid == IID_IUnknown || riid == IID_IUIAutomationEventHandler)
        {
            // Increment the reference count and return the pointer.
            *ppvObj = (LPVOID)this;
            AddRef();
            return NOERROR;
        }
        return E_NOINTERFACE;
}


ULONG AutomationEventHandle::AddRef()
{
    InterlockedIncrement(&_cRef);
    return _cRef;
}

ULONG AutomationEventHandle::Release()
{
    // Decrement the object's internal counter.
        ULONG ulRefCount = InterlockedDecrement(&_cRef);
        if (0 == _cRef)
        {
            delete this;
        }
        return ulRefCount;
}

QString AutomationEventHandle::getText()
{
    return getElementText(_lastElement);
}
