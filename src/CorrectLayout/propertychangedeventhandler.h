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

    HRESULT QueryInterface(REFIID riid, LPVOID *ppvObj);
    ULONG AddRef();
    ULONG Release();

    QMutex mutex;
private:
    void* _cl;
    ULONG _cRef;
    IUIAutomation* _automation;
    HRESULT HandlePropertyChangedEvent(IUIAutomationElement *sender, PROPERTYID propertyId, VARIANT newValue);
    IUIAutomationElement* findElement(IUIAutomationElement* element);
    QString getElementText(IUIAutomationElement* element);
};

#endif // PROPERTYCHANGEDEVENTHANDLER_H
