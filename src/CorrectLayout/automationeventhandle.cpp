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
    _keyboardFocus = NULL;
}

HRESULT AutomationEventHandle::updateText(IUIAutomationElement *element)
{
    _keyboardFocus = element;
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
    IUIAutomationCondition* isValueReadOnly;
    IUIAutomationCondition* notPassword;
    IUIAutomationCondition* textEditable;
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
    hr = _automation->CreatePropertyCondition(UIA_ValueIsReadOnlyPropertyId, falseVar, &isValueReadOnly);
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
    _automation->CreateAndCondition(textHasKeyboardFocus, isValueReadOnly, &textEditable);
    if (FAILED(hr))
    {
        qDebug() << "Failed to CreateAndCondition";
        return NULL;
    }
    _automation->CreateAndCondition(textEditable, notPassword, &searchCondition);
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
    }
    return keyboardFocus;
}

QString AutomationEventHandle::getElementSelection(IUIAutomationElement *element)
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
    IUIAutomationTextRangeArray* selections;
    atp->GetSelection(&selections);
    if (FAILED(hr))
    {
        qDebug() << "Failed to get document range.";
        return "";
    }
    int len = -1;
    if(selections == NULL || selections->get_Length(&len) > 0)
    {
        qDebug() << "There is no selection, or there is more than 1 selection";
        return "";
    }
    hr = selections->GetElement(0, &atr);
    if (FAILED(hr) || atp == NULL)
    {
        qDebug() << "Failed to get text range.";
        return "";
    }
    atr->GetText(300, &bstr);
    if (FAILED(hr) || bstr == 0x0)
    {
        qDebug() << "Failed to get element text.";
        return "";
    }
    str = QString::fromStdWString(bstr);
    return str;
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

    VARIANT isText;
    _keyboardFocus->GetCurrentPropertyValue(UIA_IsTextPatternAvailablePropertyId, &isText);
    if(!isText.boolVal)
    {
        return "";
    }
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
    if (FAILED(hr) || bstr == 0x0)
    {
        qDebug() << "Failed to get element text.";
        return "";
    }
    str = QString::fromStdWString(bstr);
    return str;
}

HRESULT AutomationEventHandle::HandleAutomationEvent(IUIAutomationElement *sender, EVENTID eventId)
{
    if(!((CorrectLayout*)_cl)->isRunning())
        return S_FALSE;

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

QString AutomationEventHandle::getSelection()
{
    return getElementSelection(findElement(_lastElement));
}
