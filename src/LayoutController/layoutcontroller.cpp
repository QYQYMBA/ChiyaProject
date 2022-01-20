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
}

LayoutController::~LayoutController()
{
    _settings.endGroup();
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

    _keyPressId = QtGlobalInput::waitForKeyPress(0, EventType::ButtonUp, &LayoutController::handleKey, this, true);
    _windowSwitchId = QtGlobalInput::setWindowSwitch(&LayoutController::windowSwitched, this);

    getLayoutSettingsList();

    loadSettings();
    if(_exceptions.length() == 0)
        removeSystemShortcut();
    windowSwitched(GetForegroundWindow()); // Simulating window switches

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

void LayoutController::handleKey(RAWKEYBOARD keyboard)
{
    if(keyboard.Message != WM_KEYUP && keyboard.Message != WM_SYSKEYUP)
        return;

    bool ctrl = GetAsyncKeyState(VK_LCONTROL);
    bool shift = GetAsyncKeyState(VK_LSHIFT);
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

    bool ralt = (keyboard.VKey == VK_MENU && keyboard.MakeCode == 56);

    bool lctrl = (keyboard.VKey == VK_CONTROL && !(keyboard.Flags & RI_KEY_E0));
    bool rctrl = (keyboard.VKey == VK_CONTROL && keyboard.Flags & RI_KEY_E0);

    bool ctrlshift = (_toggleValue == 2 && (lshift || rshift || lctrl || rctrl));
    bool shiftalt  = (_toggleValue == 1 && (lshift || rshift || ralt));

    bool secondbuttonpressed = (_toggleValue == 2 && ctrl ) || (_toggleValue == 1 && GetAsyncKeyState(VK_LMENU));

    if( ctrlshift || shiftalt )
        if( shift &&  secondbuttonpressed )
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

            QString windowName(WinApiAdapter::GetWindowName(newParent));

            QString className(WinApiAdapter::GetWindowClass(newParent));

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

            if(className == "ConsoleWindowClass")
            {
               _currentLayout = _correctLayout;
            }
            else if(newParent == _oldParent)
            {
                _currentLayout = WinApiAdapter::GetLayoutByHwnd(_rightChild);
            }
            else
            {
                _currentLayout = WinApiAdapter::GetLayoutByHwnd(newParent);
            }
            _oldParent = newParent;

            int newLayout = 0;
            for(int i = 0; i < _layoutsSettings.size(); i++)
            {
                if(_currentLayout == _layoutsSettings[i].layout)
                {
                    newLayout = i + 1;
                }
            }
            if(!_changeRegistry)
                Sleep(10);

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

            _rightChild = newParent;
            Sleep(1);
            EnumChildWindows( newParent, EnumChildProc, (LPARAM)this);
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

            QString layout = QString::fromStdString(WinApiAdapter::hklToStr(layoutsList[i]));
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
