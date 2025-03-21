#include "layoutcontroller.h"

#include <sstream>

#include <QAbstractEventDispatcher>
#include <QCoreApplication>

#include "key.h"
#include "adminrights.h"
#include "winapiadapter.h"

LayoutController::LayoutController(HWND hwnd)
{
    _settings.beginGroup("LayoutController");

    _toggleValue = 1;

    _running = false;

    _myHWND = hwnd;

    _shell = FindWindow(L"Shell_TrayWnd", 0);

    findDesktopHanlde();

    _currentLayout = WinApiAdapter::GetLayoutByHwnd(GetForegroundWindow());
    _correctLayout = _currentLayout;
    _externalShortcutPressed = false;
}

LayoutController::~LayoutController()
{
    _settings.endGroup();
    stop();
}

BOOL EnumChildProc( HWND hwnd, LPARAM lParam )
{
    HKL childLayout = GetKeyboardLayout(GetWindowThreadProcessId(hwnd, NULL));
    LayoutController* lc = (LayoutController*) lParam;

    for(int i = 0; i < lc->_layoutsSettings.size(); i++)
    {
        if(childLayout == lc->_layoutsSettings[i].layout)
        {
            if(lc->_layoutsSettings[i].active == false)
                return true;
        }
    }

    if(lc->_currentLayout != childLayout && lc->_correctLayout == childLayout)
    {
        lc->_rightChild = hwnd;
        return false;
    }
    return true;
}

BOOL EnumAllProc( HWND hwnd, LPARAM lParam )
{
    QString className(WinApiAdapter::GetWindowClass(hwnd));

    QString windowName(WinApiAdapter::GetWindowName(hwnd));

    if(className == "SHELLDLL_DefView" && windowName == "")
    {
        LayoutController* lc = (LayoutController*) lParam;
        lc->_desktop = GetParent(hwnd);
        return false;
    }
    return true;
}

void LayoutController::findDesktopHanlde()
{
    EnumChildWindows(GetDesktopWindow(), EnumAllProc, (LPARAM)this);
}

void LayoutController::loadSettings()
{
    if(AdminRights::IsRunAsAdministrator())
        _changeRegistry = _settings.value("changeRegistry").toBool();
    else
        _changeRegistry = false;

    _enableQt = _settings.value("enableQtApps").toBool();

    _whiteList = _settings.value("exceptions/isWhiteList").toBool();

    getExceptionsList();
}

bool LayoutController::start()
{
    _layoutsSettings.clear();
    _exceptions.clear();

    if(_running)
    {
        return false;
    }

    _keyPressId = QtGlobalInput::waitForKeyPress(0, EventType::All, &LayoutController::handleKey, this, true);
    _windowSwitchId = QtGlobalInput::setWindowSwitch(&LayoutController::windowSwitched, this);

    getLayoutSettingsList();

    loadSettings();
    if(_exceptions.length() == 0)
        removeSystemShortcut();

    windowSwitched(GetForegroundWindow()); // Simulating window switch

    _running = true;

    return true;
}


bool LayoutController::stop()
{
    if(!_running)
        return false;

    QtGlobalInput::removeKeyPress(_keyPressId);
    QtGlobalInput::removeWindowSwitch(_windowSwitchId);

    setSystemShortcut();

    _running = false;

    return true;
}

bool LayoutController::isRunning()
{
    return _running;
}

bool LayoutController::qtEnabled()
{
    return _enableQt;
}

HKL LayoutController::getLayout()
{
    if(_running)
        return getLayout(_oldParent);
    else
        return 0;
}

bool LayoutController::isRegistryChanged()
{
    return _registryChanged;
}

void LayoutController::switchLayout(HKL layout)
{
    HWND newParent = getForeground();

    if(!_exceptions.empty())
    {
        QString windowExeName(WinApiAdapter::GetWindowExeName(newParent));

        if(!_whiteList)
        {
            for(int i = 0; i < _exceptions.size(); i++)
            {
                if(windowExeName.toLower().contains(_exceptions[i].toLower()))
                {
                    return;
                }
            }
        }
        else
        {
            bool find = false;
            for(int i = 0; i < _exceptions.size(); i++)
            {
                if(windowExeName.toLower().contains(_exceptions[i].toLower()))
                {
                    find = true;
                    break;
                }
            }
            if(!find)
            {
                return;
            }
        }
    }

    //In Qt apps WM_INPUTLANGCHANGEREQUEST causes lags
    if(!_enableQt)
    {
        QString windowClassName(WinApiAdapter::GetWindowClass(GetForegroundWindow()));
        if (windowClassName.contains("Qt") && windowClassName.contains("QWindow"))
        {
            return;
        }
    }

    if(!_changeRegistry)
        Sleep(10);

    _currentLayout = getLayout(newParent);

    _oldParent = newParent;

    if(layout == 0)
    {
        int newLayout = 0;
        for(int i = 0; i < _layoutsSettings.size(); i++)
        {
            if(_currentLayout == _layoutsSettings[i].layout)
            {
                if(_changeRegistry)
                    newLayout = i + 1;
                else
                    newLayout = i;
                break;
            }
        }

        for(int i = 0; i < _layoutsSettings.size(); i++)
        {
            if(newLayout >= _layoutsSettings.size())
            {
                newLayout = 0;
            }
            if(!_layoutsSettings[newLayout].active)
            {
                newLayout++;
            }
            else
            {
                _correctLayout = _layoutsSettings[newLayout].layout;
                WinApiAdapter::SetKeyboardLayout(_layoutsSettings[newLayout].layout);
                break;
            }
        }
    }
    else
    {
        _correctLayout = layout;
        WinApiAdapter::SetKeyboardLayout(layout);
    }

    _rightChild = newParent;
    Sleep(1);
    EnumChildWindows(newParent, EnumChildProc, (LPARAM)this);
}

void LayoutController::removeSystemShortcut()
{
    if(_registryChanged)
        return;

    QSettings systemSettings("HKEY_CURRENT_USER\\Keyboard Layout\\Toggle", QSettings::Registry64Format);
    _toggleValue = systemSettings.value("Language Hotkey").toInt();

    QString appName = QCoreApplication::applicationName();

    QSettings appSettings("HKEY_CURRENT_USER\\SOFTWARE\\" + appName, QSettings::Registry64Format);
    if(_toggleValue < 1 || _toggleValue > 2)
        _toggleValue = appSettings.value("System Language Hotkey").toInt();
    appSettings.setValue("System Language Hotkey", _toggleValue);

    QString newValue = QString::number(3);

    if(_changeRegistry)
    {
        systemSettings.setValue("Language Hotkey", newValue);
        _registryChanged = true;
    }
}

void LayoutController::setSystemShortcut()
{
    if(!_changeRegistry)
        return;

    QString appName = QCoreApplication::applicationName();

    QSettings appSettings("HKEY_CURRENT_USER\\SOFTWARE\\" + appName, QSettings::Registry64Format);
    QString oldValue = appSettings.value("System Language Hotkey").toString();

    QSettings systemSettings("HKEY_CURRENT_USER\\Keyboard Layout\\Toggle", QSettings::Registry64Format);
    if(systemSettings.value("Language Hotkey") == 3)
        systemSettings.setValue("Language Hotkey", oldValue);

    _registryChanged = false;
}

HKL LayoutController::getLayout(HWND newParent)
{
    QString className(WinApiAdapter::GetWindowClass(newParent));

    if(className == "ConsoleWindowClass")
    {
        return _correctLayout;
    }
    else if(newParent == _oldParent)
    {
        return WinApiAdapter::GetLayoutByHwnd(_rightChild);
    }
    else
    {
        return WinApiAdapter::GetLayoutByHwnd(newParent);
    }
}

HWND LayoutController::getForeground()
{
    HWND newParent = GetForegroundWindow();

    if(newParent == _shell)
    {
        newParent = _desktop;

        INPUT altUp = WinApiAdapter::MakeKeyInput(VK_LMENU, false);
        SendInput(1, &altUp, sizeof(INPUT));


        SetForegroundWindow(_desktop);
        Sleep(1);
    }

    return newParent;
}

void LayoutController::handleKey(RAWKEYBOARD keyboard)
{
    if(!_running)
        return;

    if(((keyboard.VKey == VK_SHIFT || keyboard.VKey == VK_CONTROL) && _toggleValue == 2) ||
       ((keyboard.VKey == VK_SHIFT || keyboard.VKey == VK_MENU) && _toggleValue == 1)){
        if(keyboard.Message == WM_KEYDOWN){
            _externalShortcutPressed = false;
        }
    }
    else{
        _externalShortcutPressed = true;
    }

    if(keyboard.Message != WM_KEYUP && keyboard.Message != WM_SYSKEYUP)
        return;

    bool ctrl = GetAsyncKeyState(VK_LCONTROL) || GetAsyncKeyState(VK_RCONTROL);
    bool shift = GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT);
    bool alt = GetAsyncKeyState(VK_LMENU);

    for(int i = 0; i < _layoutsSettings.size(); i++)
    {
        if(_layoutsSettings[i].shortcutActivate.vkCode == keyboard.VKey && _layoutsSettings[i].shortcutActivate.vkCode != 0)
            if(_layoutsSettings[i].shortcutActivate.ctrl == ctrl)
                if(_layoutsSettings[i].shortcutActivate.shift == shift)
                    if(_layoutsSettings[i].shortcutActivate.alt == alt)
                    {
                        _layoutsSettings[i].active = !_layoutsSettings[i].active;
                        QString s = QString::number(reinterpret_cast<long long>(_layoutsSettings[i].layout));
                        _settings.setValue("layouts/" + s + "/deactivated", !_layoutsSettings[i].active);
                    }

        if(_layoutsSettings[i].shortcutSelect.vkCode == keyboard.VKey && _layoutsSettings[i].shortcutSelect.vkCode != 0)
            if(_layoutsSettings[i].shortcutSelect.ctrl == ctrl)
                if(_layoutsSettings[i].shortcutSelect.shift == shift)
                    if(_layoutsSettings[i].shortcutSelect.alt == alt)
                    {
                        WinApiAdapter::SetKeyboardLayout(_layoutsSettings[i].layout);
                    }
    }

    bool lshift = (keyboard.VKey == VK_SHIFT && keyboard.MakeCode == 42);
    bool rshift = (keyboard.VKey == VK_SHIFT && keyboard.MakeCode == 54);

    bool lalt = (keyboard.VKey == VK_MENU && keyboard.MakeCode == 56 && !(keyboard.Flags & RI_KEY_E0));

    bool lctrl = (keyboard.VKey == VK_CONTROL && !(keyboard.Flags & RI_KEY_E0));
    bool rctrl = (keyboard.VKey == VK_CONTROL && (keyboard.Flags & RI_KEY_E0));

    bool ctrlshift = (_toggleValue == 2 && (lshift || rshift || lctrl || rctrl));
    bool shiftalt  = (_toggleValue == 1 && (lshift || rshift || lalt));

    bool secondbuttonpressed = (_toggleValue == 2 && (ctrl || lctrl || rctrl) ) || (_toggleValue == 1 && (alt || lalt));

    if(!_externalShortcutPressed)
    {
        if( ctrlshift || shiftalt )
            if( (shift || rshift || lshift) &&  secondbuttonpressed )
            {
                    switchLayout(0);
            }
    }
}

void LayoutController::windowSwitched(HWND hwnd)
{
    if(!_exceptions.empty())
    {
        QString windowExeName(WinApiAdapter::GetWindowExeName(GetForegroundWindow()));

        if(!_whiteList)
        {
            for(int i = 0; i < _exceptions.size(); i++)
            {
                if(windowExeName.toLower().contains(_exceptions[i].toLower()))
                {
                    setSystemShortcut();
                    return;
                }
            }
            removeSystemShortcut();
        }
        else
        {
            bool find = false;
            for(int i = 0; i < _exceptions.size(); i++)
            {
                if(windowExeName.toLower().contains(_exceptions[i].toLower()))
                {
                    find = true;
                    break;
                }
            }
            if(!find)
            {
                setSystemShortcut();
                return;
            }
            removeSystemShortcut();
        }
    }

    //In Qt apps WM_INPUTLANGCHANGEREQUEST causes lags
    if(!_enableQt)
    {
        QString windowClassName(WinApiAdapter::GetWindowClass(GetForegroundWindow()));
        if (windowClassName.contains("Qt") && windowClassName.contains("QWindow"))
        {
            setSystemShortcut();
        }
        else
        {
            removeSystemShortcut();
        }
    }
}

void LayoutController::getExceptionsList()
{
    QString exceptionsString = _settings.value("exceptions/blacklist").toString();
    if(exceptionsString.size() > 0)
        _exceptions = exceptionsString.split(" ");
}

void LayoutController::getLayoutSettingsList()
{
    _layoutsSettings.clear();
    std::vector<HKL> layoutsList = WinApiAdapter::getLayoutsList();

    for (int i = 0; i < layoutsList.size(); i++)
    {
        if(layoutsList[i] != 0)
        {
            LayoutSettings ls;

            QString layout = WinApiAdapter::hklToStr(layoutsList[i]);
            ls.active = !_settings.value("layouts/" + layout + "/deactivated").toBool();

            ls.layout = layoutsList[i];

            if(_settings.value("layouts/" + layout + "/shortcut/activate/active").toBool())
            {
                Key key;
                key.vkCode = _settings.value("layouts/" + layout + "/shortcut/activate/vkCode").toInt();
                key.ctrl = _settings.value("layouts/" + layout + "/shortcut/activate/ctrl").toBool();
                key.shift = _settings.value("layouts/" + layout + "/shortcut/activate/shift").toBool();
                key.alt = _settings.value("layouts/" + layout + "/shortcut/activate/alt").toBool();
                ls.shortcutActivate = key;
            }
            else
            {
                Key key;
                key.vkCode = 0;
                ls.shortcutActivate = key;
            }

            if(_settings.value("layouts/" + layout + "/shortcut/select/active").toBool())
            {
                Key key;
                key.vkCode = _settings.value("layouts/" + layout + "/shortcut/select/vkCode").toInt();
                key.ctrl = _settings.value("layouts/" + layout + "/shortcut/select/ctrl").toBool();
                key.shift = _settings.value("layouts/" + layout + "/shortcut/select/shift").toBool();
                key.alt = _settings.value("layouts/" + layout + "/shortcut/select/alt").toBool();
                ls.shortcutSelect = key;
            }
            else
            {
                Key key;
                key.vkCode = 0;
                ls.shortcutSelect = key;
            }

            _layoutsSettings.push_back(ls);
        }
    }
}
