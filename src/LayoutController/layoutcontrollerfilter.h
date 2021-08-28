#ifndef LAYOUTCONTROLLERFILTER_H
#define LAYOUTCONTROLLERFILTER_H

#include <QAbstractNativeEventFilter>

#include "windows.h"

class LayoutControllerFilter : public QAbstractNativeEventFilter
{
public:
    LayoutControllerFilter(void* _layoutController);

    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
private:
    static void sendKeyInput(void* lc, tagRAWKEYBOARD input);

    void* _layoutController;
};

#endif // LAYOUTCONTROLLERFILTER_H
