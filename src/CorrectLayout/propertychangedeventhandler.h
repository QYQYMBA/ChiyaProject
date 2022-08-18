#ifndef PROPERTYCHANGEDEVENTHANDLER_H
#define PROPERTYCHANGEDEVENTHANDLER_H

#include "UIAutomationClient.h"
#include "UIAutomationCore.h"
#include "UIAutomationCoreApi.h"
#include "QString"
#include "QMutex"

class PropertyChangedEventHandler : IUIAutomationPropertyChangedEventHandler
{
public:
    PropertyChangedEventHandler(IUIAutomation* automation, void* _cl);

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
    HRESULT HandlePropertyChangedEvent(IUIAutomationElement *sender, PROPERTYID propertyId, VARIANT newValue);
    IUIAutomationElement* findElement(IUIAutomationElement* element);
    IUIAutomationElement* _lastElement;
    QString getElementText(IUIAutomationElement* element);
};

#endif // PROPERTYCHANGEDEVENTHANDLER_H
