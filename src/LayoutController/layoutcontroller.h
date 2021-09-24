#ifndef LAYOUTCONTROLLER_H
#define LAYOUTCONTROLLER_H

#include <windows.h>
#include <vector>
#include <string>

#include <QSettings>

#include "key.h"
#include "qtglobalinput.h"

struct LayoutSettings
{
    HKL layout;
    bool active;
    Key shortcutActivate;
    Key shortcutSelect;
};

class LayoutController
{
public:
    LayoutController(HWND hwnd);
    ~LayoutController();

    static std::vector<HKL> getLayoutsList();
    static std::string hklToStr(HKL hkl);
    static void SetKeyboardLayout(HKL layout);

    bool start();
    bool stop();
    bool isRunning();

    void handleKey(tagRAWKEYBOARD keyboard);
private:
    void loadSettings();
    void getExceptionsList();
    bool setupRawInputDevice();
    void getLayoutSettingsList();
    void findDesktopHanlde();

    void removeSystemShortcut();
    void setSystemShortcut();

    HKL GetLayout(HWND hwnd);
    friend BOOL EnumChildProc( HWND hwnd, LPARAM lParam );
    friend BOOL EnumAllProc( HWND hwnd, LPARAM lParam );
    friend INPUT MakeKeyInput(int vkCode, bool down);

    QSettings _settings;

    HWND _myHWND;

    QVector<LayoutSettings> _layoutsSettings;
    QtGlobalInput _qtGlobalInput;

    QStringList _exceptions;
    bool _whiteList = false;

    short _toggleValue;
    bool _changeRegistry;
    bool _registryChanged = false;
    bool _wasConsole = false;

    bool _running;

    HKL _currentLayout;
    HKL _correctLayout;

    HWND _oldParent;
    HWND _rightChild;

    HWND _shell;
    HWND _desktop;
};

#endif // LAYOUTCONTROLLER_H
