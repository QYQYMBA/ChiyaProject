#ifndef PROPERTYCHANGEDEVENTHANDLER_H
#define PROPERTYCHANGEDEVENTHANDLER_H

#include "UIAutomationClient.h"
#include "UIAutomationCore.h"
#include "UIAutomationCoreApi.h"
#include "QString"

class PropertyChangedEventHandler : IUIAutomationPropertyChangedEventHandler
{
public:
    PropertyChangedEventHandler(IUIAutomation* automation);

    HRESULT QueryInterface(REFIID riid, LPVOID *ppvObj);
    ULONG AddRef();
    ULONG Release();
private:
    ULONG _cRef;
    IUIAutomation* _automation;
    HRESULT HandlePropertyChangedEvent(IUIAutomationElement *sender, PROPERTYID propertyId, VARIANT newValue);
    QString getElementText(IUIAutomationElement* element);
};

#endif // PROPERTYCHANGEDEVENTHANDLER_H
