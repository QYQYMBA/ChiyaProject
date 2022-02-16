#ifndef CORRECTLAYOUT_H
#define CORRECTLAYOUT_H

#include <QString>
#include <QSettings>
#include <QtConcurrent/QtConcurrent>

#include "windows.h"

#include "layoutchecker.h"
#include "qtglobalinput.h"
#include "key.h"
#include "keypress.h"
#include "layoutcontroller.h"
#include "UIAutomation.h"
#include "llkeyhandler.h"

struct LayoutSwitchSettings
{
    HKL layout;
    bool active;
    Key shortcutActivate;
    Key shortcutSelect;
};

class CorrectLayout : public QThread
{
public:
    CorrectLayout(HWND hwnd, LayoutController* _layoutContoller);
    ~CorrectLayout();

    bool startCl();
    bool stopCl();
    bool reinitialize();
    bool init();
    bool isRunning();

    void handleKey(RAWKEYBOARD rk);
    bool handleLlKey(int nCode, WPARAM wParam, LPARAM lParam);
    void handleMouse(RAWMOUSE mouse);
    void windowSwitched(HWND hwnd);
private:
    enum class SwitcherState {SEARCHING, WORKING, CHANGING, PAUSED, STOPED} _state;

    void loadSettings();
    void getExceptionsList();
    void getLayoutSettingsList();

    void convertSelection();
    void convertCurrentWord(HKL layout);
    void checkLayout(const bool beforeKeyPress, const bool finished);

    void addKeyToQueue(KeyPress kp);

    IUIAutomationElement* getFocusedElement();
    QString getElementText(IUIAutomationElement* element);
    bool compareElements(IUIAutomationElement *element1, IUIAutomationElement *element2);

    LayoutController* _layoutController;
    LlKeyHandler _llKeyHandler;

    QVector<KeyPress> _keyPresses;
    QString _currentWord;
    QString _currentText;
    int _position;

    HKL _lastLayout;
    HKL _previousLayout;

    QVector<LayoutSwitchSettings> _layoutsSettings;
    QStringList _exceptions;

    bool _whiteList;

    uint _keyPressId;
    uint _keyLlPressId;
    uint _mousePressId;
    uint _windowSwitchId;

    QQueue<KeyPress> _keyQueue;
    KeyPress _keyToProcess;
    QTimer timer;

    IUIAutomation *_automation;
    IUIAutomationElement *_currentElement;

    LayoutChecker _layoutChecker;

    QSettings _settings;

    HWND _myHWND;

    bool _running;
    bool _initialized;
    bool _exception;

    QMutex _queueMutex;

private slots:
    void onKeyProcessed();
protected:
    void run();
};

#endif // CORRECTLAYOUT_H
