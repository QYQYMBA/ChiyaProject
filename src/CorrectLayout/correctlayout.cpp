#include "correctlayout.h"

#include <QDebug>
#include <QCoreApplication>
#include <string>
#include <iostream>
#include <fstream>
#include <queue>

#include "winapiadapter.h"

const int MAXNAMELENGTH = 30;

CorrectLayout::CorrectLayout(HWND hwnd, LayoutController* layoutController)
    : timer(),
      _layoutChecker()
{
    _settings.beginGroup("CorrectLayout");

    _running = false;
    _initialized = false;

    _layoutController = layoutController;
    _myHWND = hwnd;
    _changeLayout = 0x0;
}

CorrectLayout::~CorrectLayout()
{
    _settings.endGroup();
}

bool CorrectLayout::startCl()
{
    if(_running)
        return false;

    if(!_layoutController->isRunning())
        return false;

    qDebug() << "Starting CorrectLayout";

    _layoutsSettings.clear();
    _exceptions.clear();

    _layoutsList = WinApiAdapter::getLayoutsList();

    getLayoutSettingsList();

    loadSettings();

    if(!_initialized)
    {
        if(!init())
            return false;
    }

    _lastLayout = _layoutController->getLayout();

    _running = true;

    qDebug() << "CorrectLayout started";

    return true;
}


bool CorrectLayout::stopCl()
{
    if(!_running)
        return false;

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

    CoInitialize(NULL);
    HRESULT hr = CoCreateInstance(CLSID_CUIAutomation8, NULL,
                                  CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation),
                                  (void**)&_automation);

    if(FAILED(hr))
        qDebug() << "Automation can not be initialized";

    _pceh = new PropertyChangedEventHandler(_automation, this);
    _fceh = new FocusChangedEventHandler(_automation, _pceh, _layoutController, _passwordCapsLock, _passwordNewLayout);
    hr = _automation->AddFocusChangedEventHandler(NULL, (IUIAutomationFocusChangedEventHandler*)_fceh);
    if(FAILED(hr))
        qDebug() << "Focus changed event handler can't be added";

    qDebug() << "Start loading dictionaries";

    _mousePressId = QtGlobalInput::waitForMousePress(0, EventType::ButtonDown, &CorrectLayout::handleMouse, this, true);
    _keyLlPressId = QtGlobalInput::setLlKeyboardHook(0, EventType::ButtonDown, &CorrectLayout::handleLlKey, this, false);
    QtGlobalInput::removeLlKeyboardHook(_keyLlPressId);
    _windowSwitchId = QtGlobalInput::setWindowSwitch(&CorrectLayout::windowSwitched, this);


    for (int i = 0; i < _layoutsList.size(); i++)
    {
        wchar_t name[MAXNAMELENGTH];
        LANGID language = (LANGID)(((UINT)_layoutsList[i]) & 0x0000FFFF);
        LCID locale = MAKELCID(language, SORT_DEFAULT);

        GetLocaleInfo(locale, LOCALE_SLANGUAGE, name, MAXNAMELENGTH);

        qDebug() << "Start loading " << QString::fromWCharArray(name) << " dictionary";

        QString path = QCoreApplication::applicationDirPath() + "/Dictionaries/" + WinApiAdapter::decToHex(_layoutsList[i]) + ".txt";
        if (QFile::exists(path))
            _layoutChecker.load(path, _layoutsList[i]);
        else
            qDebug() << "No dictionary file!";
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

void CorrectLayout::windowSwitched(HWND hwnd)
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
    else
    {
        _exception = false;
    }
}

void CorrectLayout::loadSettings()
{
    _whiteList = _settings.value("exceptions/isWhiteList").toBool();

    qDebug() << "Whitelist mode:" << (_whiteList ? "True" : "False");

    _exception = false;
    getExceptionsList();

    _passwordCapsLock = _settings.value("passwords/layout").toBool();
    _passwordLayout = _settings.value("passwords/layoutChecked").toBool();
    _passwordNewLayout = 0x0;
    if (_passwordLayout)
    {
        QString layout = _settings.value("passwords/layout").toString();
        for (int i = 0; i < _layoutsList.size(); i++)
        {
            if(WinApiAdapter::hklToStr(_layoutsList[i]) == layout)
            {
                _passwordNewLayout = _layoutsList[i];
            }
        }
    }
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

            QString layout = WinApiAdapter::hklToStr(layoutsList[i]);
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

void CorrectLayout::convertCurrentWord(QString changedText)
{
    _fceh->changeValue(changedText, _position);
}

void CorrectLayout::checkLayout(const bool finished)
{
    if (_state == SwitcherState::WORKING) {
        QString wordToCheck = _currentWord;
        HKL layout = _layoutChecker.checkLayout(wordToCheck, finished);
        if (layout != nullptr && layout != _lastLayout) {
            _changeLayout = layout;
            _keyLlPressId = QtGlobalInput::setLlKeyboardHook(0, EventType::ButtonDown, &CorrectLayout::handleLlKey, this, false);
        }
        else
        {
            _changeLayout = 0x0;
        }
    }
}

void CorrectLayout::handleValueChange(QString newText)
{
    if(_exception)
        return;

    HKL newLayout = _layoutController->getLayout();
    if(newLayout != _lastLayout)
    {
        _state = SwitcherState::PAUSED;
        _lastLayout = newLayout;
    }

    _changeLayout = 0x0;

    newText = newText.replace('\n', ' ').replace('\r', ' ');
    int positionInText = 0;
    bool t = true;
    if(_fceh->elementChanged(true)){
        _state = SwitcherState::SEARCHING;
    }
    QStringList oldWords = _currentText.split(' ');
    QStringList newWords = newText.split(' ');
    QStringList changedWords;
    if(newWords.size() == oldWords.size())
    {
        for(int i = 0; i < newWords.size(); i++)
        {
            if(newWords[i] != oldWords[i])
            {
                changedWords.push_back(oldWords[i]);
                changedWords.push_back(newWords[i]);
                t = false;
            }
            if (t)
                positionInText += newWords[i].size() + 1;
        }
        if(changedWords.size() != 2)
        {
            _state = SwitcherState::SEARCHING;
        }
        else
        {
            if(_state == SwitcherState::SEARCHING)
            {
                QString oldWord = changedWords[0];
                QString newWord = changedWords[1];
                if(newWord.mid(0, newWord.length()-1) == oldWord)
                {
                    _position = positionInText + newWord.length();
                    _state = SwitcherState::WORKING;
                }
            }
            else if(_state == SwitcherState::WORKING)
            {
                QString oldWord = changedWords[0];
                QString newWord = changedWords[1];
                if(newWord.mid(0, newWord.length()-1) == oldWord)
                {
                    _position = positionInText + newWord.length();

                    _currentText = newText;
                    _currentWord = newWord;
                    if (_currentWord.length() > 2) {
                        checkLayout(false);
                    }
                }
                else
                {
                    _state = SwitcherState::SEARCHING;
                }
            }
        }
    }
    else
    {
        _state = SwitcherState::SEARCHING;
    }

    _currentText = newText;
}

bool CorrectLayout::handleLlKey(int nCode, WPARAM wParam, LPARAM lParam)
{
    PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT) lParam;

    if (key != nullptr) {
        unsigned long vkCode = key->vkCode;
        if(_layoutChecker.vkToChar(_changeLayout, vkCode, false) != NULL)
            if (!_fceh->elementChanged(false) && _changeLayout != 0x0)
            {
                int start = _position - _currentWord.size();
                int end = _currentText.indexOf(' ', start);
                if (end == -1)
                    end = _currentText.size();
                QString changedWord = _currentText.mid(start, end-start);
                _layoutChecker.changeWordLayout(changedWord, _changeLayout);
                if(changedWord.mid(0, changedWord.length()-1) != _currentWord)
                {
                    QString changedText = _currentText.replace(start, end-start, changedWord);
                    _layoutController->switchLayout(_changeLayout);
                    convertCurrentWord(changedText);
                    _currentWord = changedWord;
                }
            }
    }

    _changeLayout = 0x0;
    QtGlobalInput::removeLlKeyboardHook(_keyLlPressId);

    return false;
}

void CorrectLayout::handleMouse(RAWMOUSE mouse)
{
    _changeLayout = 0x0;
    QtGlobalInput::removeLlKeyboardHook(_keyLlPressId);
}
