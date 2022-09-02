#ifndef FOCUSCHANGEDEVENTHANDLER_H
#define FOCUSCHANGEDEVENTHANDLER_H

#include "UIAutomation.h"
#include "UIAutomationClient.h"
#include "UIAutomationCore.h"
#include "UIAutomationCoreApi.h"
#include "QString"
#include "layoutcontroller.h"
#include "automationeventhandle.h"

class FocusChangedEventHandler : IUIAutomationFocusChangedEventHandler
{
public:
    FocusChangedEventHandler(IUIAutomation* automation, AutomationEventHandle* pceh, LayoutController* layoutController, bool passwordCapsLock = false, HKL passwordLayout = 0x0);
    bool elementChanged(bool reset);
    void newElement();

    HRESULT QueryInterface(REFIID riid, LPVOID *ppvObj);
    HRESULT activateTextChangedHandler(bool state);
    ULONG AddRef();
    ULONG Release();
private:
    bool _passwordCapsLock;
    HKL _passwordLayout;

    AutomationEventHandle* _aeh;
    bool _elementChanged;
    ULONG _cRef;
    IUIAutomation* _automation;
    IUIAutomationElement* _keyboardFocus;
    LayoutController* _layoutController;

    HRESULT HandleFocusChangedEvent(IUIAutomationElement *sender);
};

#endif // FOCUSCHANGEDEVENTHANDLER_H
