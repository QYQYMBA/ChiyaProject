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
#include "focuschangedeventhandler.h"
#include "automationeventhandle.h"

struct LayoutCorrectionSettings
{
    HKL layout;
    bool active;
    bool check;
    Key shortcutActivate;
    Key shortcutSelect;
};

class CorrectLayout : public QObject
{
    Q_OBJECT
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
    void handleKey(RAWKEYBOARD keyboard);
    bool handleLlKey(int nCode, WPARAM wParam, LPARAM lParam);
    void handleMouse(RAWMOUSE mouse);
private:
    enum class SwitcherState {SEARCHING, WORKING, CHANGING, PAUSED, STOPED} _state;

    void loadSettings();
    void getExceptionsList();
    void getLayoutSettingsList();

    void convertSelection(HKL newLayout);
    QString convertCurrentWord(QString word);
    void checkLayout(const bool finished);

    bool loadDictionaries();

    LayoutController* _layoutController;

    QString _currentWord;
    QString _currentText;
    int _position;

    HKL _lastLayout;
    HKL _previousLayout;

    Key _shortcutPause;
    Key _shortcutNext;
    Key _shortcutUndo;
    QVector<LayoutCorrectionSettings> _layoutsSettings;
    QStringList _exceptions;

    bool _whiteList;

    uint _keyLlPressId;
    uint _keyPressId;
    uint _mousePressId;
    uint _windowSwitchId;

    QQueue<KeyPress> _keyQueue;
    KeyPress _keyToProcess;
    QTimer timer;

    IUIAutomation *_automation;
    FocusChangedEventHandler* _fceh;
    AutomationEventHandle* _aeh;

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
    HKL _changeLayoutSpace;
    std::vector<HKL> _layoutsList;

    HKL _oldLayout;
};

#endif // CORRECTLAYOUT_H
