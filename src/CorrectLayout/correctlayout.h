#ifndef CORRECTLAYOUT_H
#define CORRECTLAYOUT_H

#include <QString>
#include <QSettings>
#include <QMutex>
#include <QtConcurrent/QtConcurrent>

#include "windows.h"

#include "layoutchecker.h"
#include "qtglobalinput.h"
#include "key.h"
#include "keypress.h"
#include "layoutcontroller.h"
#include "UIAutomation.h"
#include "llkeyhandler.h"
#include "focuschangedeventhandler.h"
#include "propertychangedeventhandler.h"

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
    CorrectLayout(HWND hwnd, LayoutController* _layoutContoller);
    ~CorrectLayout();

    bool startCl();
    bool stopCl();
    bool reinitialize();
    bool init();
    bool isRunning();

    void windowSwitched(HWND hwnd);

    void handleValueChange(QString newString);
private:
    enum class SwitcherState {SEARCHING, WORKING, CHANGING, PAUSED, STOPED} _state;

    void loadSettings();
    void getExceptionsList();
    void getLayoutSettingsList();

    void convertSelection();
    void convertCurrentWord(QString changedText);
    void checkLayout(const bool finished);

    //void handleKeyAsync();

    LayoutController* _layoutController;

    QString _currentWord;
    QString _currentText;
    int _position;

    HKL _lastLayout;
    HKL _previousLayout;

    QVector<LayoutSwitchSettings> _layoutsSettings;
    QStringList _exceptions;

    bool _whiteList;

    uint _windowSwitchId;

    QQueue<KeyPress> _keyQueue;
    KeyPress _keyToProcess;
    QTimer timer;

    IUIAutomation *_automation;
    FocusChangedEventHandler* _fceh;
    PropertyChangedEventHandler* _pceh;

    LayoutChecker _layoutChecker;

    QSettings _settings;

    HWND _myHWND;

    bool _running;
    bool _initialized;
    bool _exception;
    bool _passwordCapsLock;
    bool _passwordLayout;
    HKL _passwordNewLayout;

    HKL _changeLayout;
    std::vector<HKL> _layoutsList;
};

#endif // CORRECTLAYOUT_H
