#ifndef FOCUSCHANGEDEVENTHANDLER_H
#define FOCUSCHANGEDEVENTHANDLER_H

#include "UIAutomationClient.h"
#include "UIAutomationCore.h"
#include "UIAutomationCoreApi.h"
#include "QString"
#include "propertychangedeventhandler.h"

class FocusChangedEventHandler : IUIAutomationFocusChangedEventHandler
{
public:
    FocusChangedEventHandler(IUIAutomation* automation);
    QString getElementText();
    bool elementChanged();

    HRESULT QueryInterface(REFIID riid, LPVOID *ppvObj);
    ULONG AddRef();
    ULONG Release();
private:
    PropertyChangedEventHandler* _pceh;
    IUIAutomationElement* _sender;
    bool _elementChanged;
    ULONG _cRef;
    IUIAutomation* _automation;
    QString getElementText(IUIAutomationElement* element);
    HRESULT HandleFocusChangedEvent(IUIAutomationElement *sender);
};

#endif // FOCUSCHANGEDEVENTHANDLER_H
