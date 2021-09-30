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
    ,_qtGlobalInput(hwnd)
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

    _layoutsSettings.clear();
    _exceptions.clear();

    if(!_initialized)
    {
        if(!init())
            return false;
    }

    _keyPressId = _qtGlobalInput.setKeyPress(0, QtGlobalInput::EventType::ButtonUp, &CorrectLayout::handleKey, this, false);
    _mousePressId = _qtGlobalInput.setMousePress(0, QtGlobalInput::EventType::ButtonDown, &CorrectLayout::handleMouse, this, false);
    _windowSwitchId = _qtGlobalInput.setWindowSwitch(&CorrectLayout::windowSwitched, this);

    getLayoutSettingsList();

    loadSettings();

    _running = true;

    return true;
}


bool CorrectLayout::stop()
{
    if(!_running)
        return false;

    _qtGlobalInput.removeKeyPress(_keyPressId);
    _qtGlobalInput.removeMousePress(_mousePressId);
    _qtGlobalInput.removeWindowSwitch(_windowSwitchId);

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

    return true;
}

bool CorrectLayout::isRunning()
{
    return _running;
}

void CorrectLayout::handleKey(RAWKEYBOARD keyboard)
{
    if (_state != SwitcherState::PAUSED) {
        Key key(keyboard, GetAsyncKeyState(VK_SHIFT), GetKeyState(VK_CAPITAL));

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

        if (key.vkCode == VK_PAUSE) {
            _state = SwitcherState::PAUSED;
            convertCurrentWord(NULL);
            _layoutChecker.changeWordLayout(_currentWord, _lastLayout);
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
            char keyChar = key.toChar();
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
}

void CorrectLayout::handleMouse(RAWMOUSE mouse)
{
    _state = SwitcherState::NORMAL;
    _currentWord = "";
}

void CorrectLayout::windowSwitched(HWND hwnd)
{

}

void CorrectLayout::loadSettings()
{
    _whiteList = _settings.value("exceptions/isWhiteList").toBool();

    getExceptionsList();
}

void CorrectLayout::getExceptionsList()
{
    QString exceptionsString = _settings.value("exceptions/blacklist").toString();
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

    for (std::string::size_type i = 0; i < word.length(); i++) {
        WinApiAdapter::SendKeyPress(VK_BACK, false);
    }

    std::queue<Key> keys;
    for (char c : word) {
        keys.push(Key::CharToKey(c, _lastLayout));
    }

    if (layout == NULL) {
        WinApiAdapter::NextKeyboardLayout();
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
