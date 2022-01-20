#include "correctlayout.h"

#include <QDebug>
#include <QCoreApplication>
#include <string>
#include <iostream>
#include <fstream>
#include <queue>

#include "winapiadapter.h"

const int MAXNAMELENGTH = 30;

CorrectLayout::CorrectLayout(HWND hwnd)
    :_layoutChecker()
{
    _settings.beginGroup("CorrectLayout");

    _running = false;

    _myHWND = hwnd;
}

CorrectLayout::~CorrectLayout()
{
    _settings.endGroup();

}

bool CorrectLayout::start()
{
    if(_running)
        return false;

    qDebug() << "Starting CorrectLayout";

    _layoutsSettings.clear();
    _exceptions.clear();

    if(!_initialized)
    {
        if(!init())
            return false;
    }

    _keyPressId = QtGlobalInput::setLlKeyboardHook(0, EventType::ButtonDown, &CorrectLayout::handleKey, this, true);
    _mousePressId = QtGlobalInput::waitForMousePress(0, EventType::ButtonDown, &CorrectLayout::handleMouse, this, true);
    _windowSwitchId = QtGlobalInput::setWindowSwitch(&CorrectLayout::windowSwitched, this);

    getLayoutSettingsList();

    loadSettings();

    _running = true;

    qDebug() << "CorrectLayout started";

    return true;
}


bool CorrectLayout::stop()
{
    if(!_running)
        return false;

    QtGlobalInput::removeKeyPress(_keyPressId);
    QtGlobalInput::removeMousePress(_mousePressId);
    QtGlobalInput::removeWindowSwitch(_windowSwitchId);

    _running = false;

    return true;
}

bool CorrectLayout::reinitialize()
{
    _layoutChecker = LayoutChecker();
    _initialized = false;
    return init();
}

bool CorrectLayout::init()
{
    if(_initialized)
        return false;

    qDebug() << "CorrectLayout initialization";

    qDebug() << "Start loading dictionaries";

    std::vector<HKL> layoutsList = WinApiAdapter::getLayoutsList();

    for (int i = 0; i < layoutsList.size(); i++)
    {
        wchar_t name[MAXNAMELENGTH];
        LANGID language = (LANGID)(((UINT)layoutsList[i]) & 0x0000FFFF);
        LCID locale = MAKELCID(language, SORT_DEFAULT);

        GetLocaleInfo(locale, LOCALE_SLANGUAGE, name, MAXNAMELENGTH);

        qDebug() << "Start loading " << QString::fromWCharArray(name) << " dictionary";
        std::string path = QCoreApplication::applicationDirPath().toStdString() + "/Dictionaries/" + WinApiAdapter::decToHex(layoutsList[i]) + ".txt";
        _layoutChecker.load(path, layoutsList[i]);
        qDebug() << "Finish loading " << QString::fromWCharArray(name) << " dictionary";
    }

    qDebug() << "Finish loading dictionaries";

    _initialized = true;

    qDebug() << "Initialization finished successfully";

    return true;
}

bool CorrectLayout::isRunning()
{
    return _running;
}

void CorrectLayout::handleKey(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (_state == SwitcherState::PAUSED)
        return;

    PKBDLLHOOKSTRUCT keyboard = (PKBDLLHOOKSTRUCT)lParam;
    Key key(keyboard, GetAsyncKeyState(VK_SHIFT), GetKeyState(VK_CAPITAL));

    bool ctrl = GetAsyncKeyState(VK_LCONTROL);
    bool shift = GetAsyncKeyState(VK_LSHIFT);
    bool alt = GetAsyncKeyState(VK_LMENU);

    if (key.vkCode == VK_F12 && GetAsyncKeyState(VK_CONTROL)) {
        if (_state == SwitcherState::FULL_PAUSED) {
            _state = SwitcherState::NORMAL;
        } else
            if (_state == SwitcherState::NORMAL) {
                _state = SwitcherState::FULL_PAUSED;
            }
    }

    if (_state == SwitcherState::FULL_PAUSED) {
        return;
    }

    if(_exception)
        return;

    if(!_exceptions.empty())
    {
        QString windowExeName(WinApiAdapter::GetWindowExeName(GetForegroundWindow()));

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

    for(int i = 0; i < _layoutsSettings.size(); i++)
    {
        if(_layoutsSettings[i].shortcutActivate.vkCode == key.vkCode && _layoutsSettings[i].shortcutActivate.vkCode != 0)
            if(_layoutsSettings[i].shortcutActivate.ctrl == ctrl)
                if(_layoutsSettings[i].shortcutActivate.shift == shift)
                    if(_layoutsSettings[i].shortcutActivate.alt == alt)
                    {
                        _layoutsSettings[i].active = !_layoutsSettings[i].active;
                        QString s;
                        s = s.fromStdString(WinApiAdapter::hklToStr(_layoutsSettings[i].layout));
                        _settings.setValue("layouts/" + s + "/deactivated", !_layoutsSettings[i].active);
                        return;
                    }

        if(_layoutsSettings[i].shortcutSelect.vkCode == key.vkCode && _layoutsSettings[i].shortcutSelect.vkCode != 0)
            if(_layoutsSettings[i].shortcutSelect.ctrl == ctrl)
                if(_layoutsSettings[i].shortcutSelect.shift == shift)
                    if(_layoutsSettings[i].shortcutSelect.alt == alt)
                    {
                        /*convertCurrentWord(_layoutsSettings[i].layout);
                        _state = SwitcherState::BLOCKED;
                        _layoutChecker.changeWordLayout(_currentWord, _layoutsSettings[i].layout);
                        return;*/
                    }
    }

    if (key.vkCode == VK_SPACE) {
        _state = SwitcherState::NORMAL;
        HKL newLayout = WinApiAdapter::GetLayout();
        if (_lastLayout != newLayout) {
            _currentWord = "";
        }
        checkLayout(false, true);
        _previousLayout = _lastLayout;
        _currentWord = "";
    }

    if(_state == SwitcherState::BLOCKED)
        return;

    if (key.vkCode == VK_BACK) {
        _currentWord = _currentWord.substr(0, _currentWord.length() - 1);
    }

    if (key.isPrintable()) {
        HKL newLayout = WinApiAdapter::GetLayout();
        if (_lastLayout != newLayout) {
            _state = SwitcherState::NORMAL;
            _currentWord = "";
        }
        _lastLayout = newLayout;
        int keyChar = key.toChar();
        std::string specialChars = "~`!@#$%^&*()_-+=[{]}'\";:,<.>/?\\|";
        int n = specialChars.find(keyChar);
        if(n > 0)
        {
            _state = SwitcherState::BLOCKED;
        }
        if (keyChar == 1) {
            _currentWord = "";
        }
        else {
            _currentWord += keyChar;
        }
        if (_currentWord.length() > 2) {
            checkLayout(true, false);
            if (key.shift == true)
            {
                INPUT input = WinApiAdapter::MakeKeyInput(VK_SHIFT, true);
                SendInput(1, &input, sizeof(INPUT));
            }
        }
    }
}

void CorrectLayout::handleMouse(RAWMOUSE mouse)
{
    _state = SwitcherState::NORMAL;
    _currentWord = "";
}

void CorrectLayout::windowSwitched(HWND hwnd)
{
    if(!_exceptions.empty())
    {
        QString windowExeName(WinApiAdapter::GetWindowExeName(GetForegroundWindow()));

        qDebug() << windowExeName;

        if(!_whiteList)
        {
            for(int i = 0; i < _exceptions.size(); i++)
            {
                if(windowExeName.toLower().contains(_exceptions[i].toLower()))
                {
                    _exception = true;
                    return;
                }
            }
            _exception = false;
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
                _exception = true;
                return;
            }
            _exception = false;
        }
    }
}

void CorrectLayout::loadSettings()
{
    _whiteList = _settings.value("exceptions/isWhiteList").toBool();

    qDebug() << "Whitelist mode:" << (_whiteList ? "True" : "False");

    getExceptionsList();
}

void CorrectLayout::getExceptionsList()
{
    QString exceptionsString = _settings.value("exceptions/blacklist").toString();
    qDebug() << "Exceptions: " + exceptionsString;
    if(exceptionsString.size() > 0)
        _exceptions = exceptionsString.split(" ");
}

void CorrectLayout::getLayoutSettingsList()
{
    _layoutsSettings.clear();
    std::vector<HKL> layoutsList = WinApiAdapter::getLayoutsList();

    for (int i = 0; i < layoutsList.size(); i++)
    {
        if(layoutsList[i] != 0)
        {
            LayoutSwitchSettings ls;

            QString layout = QString::fromStdString(WinApiAdapter::hklToStr(layoutsList[i]));
            ls.active = !_settings.value("layouts/" +layout + "/deactivated").toBool();

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

void CorrectLayout::convertCurrentWord(HKL layout)
{
    std::string word = _currentWord;

    if (word.empty())
        return;

    _state = SwitcherState::PAUSED;

    /*for (std::string::size_type i = 0; i < word.length(); i++) {
        WinApiAdapter::SendKeyPress(VK_BACK, false);
    }*/

    WinApiAdapter::SendKeyPress(VK_BACK, false, true, false);

    std::queue<Key> keys;
    for (char c : word) {
        keys.push(Key::CharToKey(c, _lastLayout));
    }

    if (layout == NULL) {

    }
    else {
        WinApiAdapter::SetKeyboardLayout(layout);
    }

    std::string newWord = "";

    while (!keys.empty()) {
        WinApiAdapter::SendKeyPress(keys.front().vkCode, keys.front().shift);
        keys.pop();
    }

    Sleep(50);

    _lastLayout = WinApiAdapter::GetLayout();

    _state = SwitcherState::NORMAL;
}

void CorrectLayout::checkLayout(const bool beforeKeyPress, const bool finished)
{
    if (_currentWord.length() > 0 && _state == SwitcherState::NORMAL) {
        std::string wordToCheck = _currentWord;
        HKL layout = _layoutChecker.checkLayout(wordToCheck, finished);
        if (layout != nullptr && layout != _lastLayout) {
            if (beforeKeyPress) {
                _currentWord = _currentWord.substr(0, _currentWord.length() - 1);
            }
            convertCurrentWord(layout);
            _currentWord = wordToCheck;
            _layoutChecker.changeWordLayout(_currentWord, _lastLayout);
        }
    }
}
