#ifndef CORRECTLAYOUT_H
#define CORRECTLAYOUT_H

#include <QString>
#include <QSettings>

#include "windows.h"
#include "UIAutomationClient.h"
#include "UIAutomationCore.h"

#include "layoutchecker.h"
#include "qtglobalinput.h"
#include "key.h"
#include "keypress.h"


struct LayoutSwitchSettings
{
    HKL layout;
    bool active;
    Key shortcutActivate;
    Key shortcutSelect;
};

class CorrectLayout
{
public:
    CorrectLayout(HWND hwnd);
    ~CorrectLayout();

    bool start();
    bool stop();
    bool reinitialize();
    bool init();
    bool isRunning();

    void handleKey(int nCode, WPARAM wParam, LPARAM lParam);
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

    IUIAutomationElement* getFocusedElement();
    QString getElementText(IUIAutomationElement* element);
    bool compareElements(IUIAutomationElement* element1, IUIAutomationElement* element2);

    IUIAutomation *_automation;
    IUIAutomationElement *_currentElement;

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
    uint _mousePressId;
    uint _windowSwitchId;

    LayoutChecker _layoutChecker;

    QSettings _settings;

    HWND _myHWND;

    bool _running;
    bool _initialized;
    bool _exception;
};

#endif // CORRECTLAYOUT_H
