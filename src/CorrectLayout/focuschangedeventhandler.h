#ifndef FOCUSCHANGEDEVENTHANDLER_H
#define FOCUSCHANGEDEVENTHANDLER_H

#include "UIAutomation.h"
#include "UIAutomationClient.h"
#include "UIAutomationCore.h"
#include "UIAutomationCoreApi.h"
#include "QString"
#include "layoutcontroller.h"
#include "propertychangedeventhandler.h"

class FocusChangedEventHandler : IUIAutomationFocusChangedEventHandler
{
public:
    FocusChangedEventHandler(IUIAutomation* automation, PropertyChangedEventHandler* pceh, LayoutController* layoutController, bool passwordCapsLock = false, HKL passwordLayout = 0x0);
    bool elementChanged(bool reset);
    HRESULT changeValue(QString newValue, int position);

    HRESULT QueryInterface(REFIID riid, LPVOID *ppvObj);
    ULONG AddRef();
    ULONG Release();
private:
    bool _passwordCapsLock;
    HKL _passwordLayout;

    PropertyChangedEventHandler* _pceh;
    bool _elementChanged;
    ULONG _cRef;
    IUIAutomation* _automation;
    IUIAutomationElement* _keyboardFocus;
    LayoutController* _layoutController;

    IUIAutomationLegacyIAccessiblePattern* avp;
    IUIAutomationTextPattern2* atp;
    IUIAutomationTextRange* atr;

    IUIAutomationElement* findElement(IUIAutomationElement* element);
    HRESULT HandleFocusChangedEvent(IUIAutomationElement *sender);
};

#endif // FOCUSCHANGEDEVENTHANDLER_H
