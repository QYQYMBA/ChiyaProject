#ifndef FOCUSCHANGEDEVENTHANDLER_H
#define FOCUSCHANGEDEVENTHANDLER_H

#include "UIAutomation.h"
#include "UIAutomationClient.h"
#include "UIAutomationCore.h"
#include "UIAutomationCoreApi.h"
#include "QString"
#include "propertychangedeventhandler.h"

class FocusChangedEventHandler : IUIAutomationFocusChangedEventHandler
{
public:
    FocusChangedEventHandler(IUIAutomation* automation, PropertyChangedEventHandler* pceh);
    bool elementChanged(bool reset);
    HRESULT changeValue(QString newValue, int position);

    HRESULT QueryInterface(REFIID riid, LPVOID *ppvObj);
    ULONG AddRef();
    ULONG Release();
private:
    PropertyChangedEventHandler* _pceh;
    bool _elementChanged;
    ULONG _cRef;
    IUIAutomation* _automation;
    IUIAutomationElement* _keyboardFocus;

    IUIAutomationLegacyIAccessiblePattern* avp;
    IUIAutomationTextPattern2* atp;
    IUIAutomationTextRange* atr;

    IUIAutomationElement* findElement(IUIAutomationElement* element);
    HRESULT HandleFocusChangedEvent(IUIAutomationElement *sender);
};

#endif // FOCUSCHANGEDEVENTHANDLER_H
