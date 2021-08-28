#include "layoutcontrollerfilter.h"

#include <QtConcurrent/QtConcurrent>

#include "layoutcontroller.h"

LayoutControllerFilter::LayoutControllerFilter(void* lc)
{
    _layoutController = lc;
}

void LayoutControllerFilter::sendKeyInput(void* lc, tagRAWKEYBOARD input)
{
    ((LayoutController*)lc)->handleKey(input);
}


bool LayoutControllerFilter::nativeEventFilter(const QByteArray &eventType, void *message, long long *result)
{
    MSG* msg = (MSG*) message;
    switch (msg->message)
    {
    case WM_INPUT:
        if( ((LayoutController*)_layoutController)->isRunning())
        {
            UINT dwSize;
            LPARAM lParam = msg->lParam;

            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
            LPBYTE lpb = new BYTE[dwSize];
            if (lpb == NULL)
            {
                return 0;
            }

            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
                qDebug() << "GetRawInputData does not return correct size !\n";

            RAWINPUT* raw = (RAWINPUT*)lpb;

            if (raw->header.dwType == RIM_TYPEKEYBOARD)
            {
                QtConcurrent::run(sendKeyInput, _layoutController, raw->data.keyboard);
            }
        }
        break;
    }
    return false;
}
