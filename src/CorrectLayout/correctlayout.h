#ifndef CORRECTLAYOUT_H
#define CORRECTLAYOUT_H

#include <QString>
#include <QSettings>

#include "windows.h"

#include "layoutchecker.h"
#include "qtglobalinput.h"
#include "key.h"


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
    enum class SwitcherState {NORMAL, FULL_PAUSED, PAUSED, BLOCKED} _state;

    void loadSettings();
    void getExceptionsList();
    void getLayoutSettingsList();

    void convertSelection();
    void convertCurrentWord(HKL layout);
    void checkLayout(const bool beforeKeyPress, const bool finished);

    std::string _currentWord;
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
