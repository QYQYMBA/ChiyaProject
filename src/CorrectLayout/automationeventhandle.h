#ifndef AUTOMATIONEVENTHANDLE_H
#define AUTOMATIONEVENTHANDLE_H

#include "UIAutomationClient.h"
#include "UIAutomationCore.h"
#include "UIAutomationCoreApi.h"
#include "QString"
#include "QMutex"

class AutomationEventHandle : IUIAutomationEventHandler
{
public:
    AutomationEventHandle(IUIAutomation* automation, void* _cl);

    HRESULT updateText(IUIAutomationElement* element);

    HRESULT QueryInterface(REFIID riid, LPVOID *ppvObj);
    ULONG AddRef();
    ULONG Release();
    QString getText();
private:
    QString _oldText;
    void* _cl;
    ULONG _cRef;
    IUIAutomation* _automation;
    HRESULT HandleAutomationEvent(IUIAutomationElement *sender, EVENTID eventId);
    IUIAutomationElement* findElement(IUIAutomationElement* element);
    IUIAutomationElement* _lastElement;
    QString getElementText(IUIAutomationElement* element);
};

#endif // AUTOMATIONEVENTHANDLE_H
