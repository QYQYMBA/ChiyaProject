#include "correctlayout.h"

#include <QDebug>
#include <QCoreApplication>
#include <string>
#include <iostream>
#include <fstream>
#include <queue>

#include "winapiadapter.h"

#include <QtConcurrent/QtConcurrent>

const int MAXNAMELENGTH = 30;

CorrectLayout::CorrectLayout(HWND hwnd)
    :_layoutChecker()
{
    _settings.beginGroup("CorrectLayout");

    _running = false;
    _initialized = false;

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

    _currentElement = NULL;
    _lastLayout = NULL;

    _keyPressId = QtGlobalInput::setLlKeyboardHook(0, EventType::ButtonDown, &CorrectLayout::handleKey, this, false);
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

    CoInitialize(NULL);
    HRESULT hr = CoCreateInstance(__uuidof(CUIAutomation), NULL,
                                  CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation),
                                  (void**)&_automation);

    if(FAILED(hr))
        return "Automation can not be initialized";

    qDebug() << "Start loading dictionaries";

    std::vector<HKL> layoutsList = WinApiAdapter::getLayoutsList();

    for (int i = 0; i < layoutsList.size(); i++)
    {
        wchar_t name[MAXNAMELENGTH];
        LANGID language = (LANGID)(((UINT)layoutsList[i]) & 0x0000FFFF);
        LCID locale = MAKELCID(language, SORT_DEFAULT);

        GetLocaleInfo(locale, LOCALE_SLANGUAGE, name, MAXNAMELENGTH);

        qDebug() << "Log: Start loading " << QString::fromWCharArray(name) << " dictionary";

        QString path = QCoreApplication::applicationDirPath() + "/Dictionaries/" + WinApiAdapter::decToHex(layoutsList[i]) + ".txt";
        _layoutChecker.load(path, layoutsList[i]);
        qDebug() << "Log: Finish loading " << QString::fromWCharArray(name) << " dictionary";
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
    if (!GetAsyncKeyState(VK_LSHIFT) && !GetAsyncKeyState(VK_RSHIFT))
    {
        INPUT input = WinApiAdapter::MakeKeyInput(VK_SHIFT, false);
        SendInput(1, &input, sizeof(INPUT));
    }

    if (_state == SwitcherState::CHANGING)
        return;

    bool ctrl = GetAsyncKeyState(VK_CONTROL);
    bool shift = GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT);
    bool alt = GetAsyncKeyState(VK_MENU);

    PKBDLLHOOKSTRUCT keyboard = (PKBDLLHOOKSTRUCT)lParam;
    KeyPress keyPress(keyboard, shift, true, ctrl, alt, WinApiAdapter::GetLayout());

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

    IUIAutomationElement* newElement = getFocusedElement();
    if(!compareElements(_currentElement, newElement))
    {
        _currentText = "";
        _currentElement = newElement;
        _state = SwitcherState::SEARCHING;
        _keyPresses.clear();
        if(keyPress.isPrintable())
            _keyPresses.push_back(keyPress);
        _currentText = getElementText(_currentElement);
    }

    if(keyPress.getVkCode() == VK_SPACE || keyPress.getVkCode() == VK_RETURN)
    {
        checkLayout(false, true);
        _currentText = "";
        _state = SwitcherState::SEARCHING;
        _keyPresses.clear();
        _currentText = getElementText(_currentElement);
    }

    if(WinApiAdapter::GetLayout() != _lastLayout && _lastLayout != NULL)
    {
        _state = SwitcherState::PAUSED;
        _lastLayout = WinApiAdapter::GetLayout();
    }

    _currentText = _currentText.replace('\n', ' ').replace('\r', ' ');

    if(_state == SwitcherState::PAUSED)
        return;

    if(_keyPresses.size() == 0 || !keyPress.isPrintable())
    {
        _keyPresses.push_back(keyPress);
        return;
    }

    if(keyPress.getVkCode() == VK_BACK)
    {
        _keyPresses.pop_back();
        _currentText = getElementText(_currentElement).replace('\n', ' ').replace('\r', ' ');;
        return;
    }

    if(_layoutChecker.isKeyInDictionary(keyPress.getVkCode()))
    {
        QString newText = getElementText(_currentElement).replace('\n', ' ').replace('\r', ' ');
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
                }
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
                    for(int i = 0; i < oldWord.size() + 1; i++)
                    {
                        QString variant = oldWord;

                        variant.insert(i, _keyPresses.back().toChar());

                        if(variant == newWord && i == newWord.size() - 1)
                        {
                            _position = i + 1;
                            _state = SwitcherState::WORKING;
                        }
                    }
                }
                else if(_state == SwitcherState::WORKING)
                {
                    QString oldWord = changedWords[0];
                    oldWord.insert(_position, _keyPresses.back().toChar());
                    oldWord = oldWord.toLower();
                    QString newWord = changedWords[1].toLower();
                    if(newWord == oldWord)
                    {
                        _position++;

                        _currentWord = newWord + keyPress.toChar();
                        if (_currentWord.length() > 2) {
                            bool needShift = GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT);
                            checkLayout(true, false);
                            if (needShift == true)
                            {
                                INPUT input = WinApiAdapter::MakeKeyInput(VK_SHIFT, true);
                                SendInput(1, &input, sizeof(INPUT));
                            }
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
        _keyPresses.push_back(keyPress);
    }
    else
    {
        _state = SwitcherState::PAUSED;
    }
}

void CorrectLayout::handleMouse(RAWMOUSE mouse)
{
    _state = SwitcherState::SEARCHING;
    _currentWord = "";
    _keyPresses.clear();
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

void CorrectLayout::convertCurrentWord(HKL layout)
{
    QString word = _currentWord;

    if (word.size() == 0)
        return;

    _state = SwitcherState::CHANGING;

    WinApiAdapter::SendKeyPress(VK_BACK, false, true, false);

    std::queue<KeyPress> keys;
    for(int i =  _keyPresses.size() - _currentWord.size(); i < _keyPresses.size(); i++)
    {
        keys.push(_keyPresses[i]);
    }

    if (layout == NULL) {

    }
    else {
        WinApiAdapter::SetKeyboardLayout(layout);
    }

    Sleep(10);

    std::string newWord = "";

    while (!keys.empty()) {
        WinApiAdapter::SendKeyPress(keys.front().getVkCode(), keys.front().isShiftPressed());
        keys.pop();
    }

    Sleep(10);

    _lastLayout = WinApiAdapter::GetLayout();

    _state = SwitcherState::WORKING;
}

void CorrectLayout::checkLayout(const bool beforeKeyPress, const bool finished)
{
    if (_currentWord.length() > 0 && _state == SwitcherState::WORKING) {
        QString wordToCheck = _currentWord;
        HKL layout = _layoutChecker.checkLayout(wordToCheck, finished);
        if (layout != nullptr && layout != _lastLayout) {
            if (beforeKeyPress) {
                _currentWord = _currentWord.mid(0, _currentWord.length() - 1);
            }
            convertCurrentWord(layout);
            _currentWord = wordToCheck;
            _layoutChecker.changeWordLayout(_currentWord, _lastLayout);
        }
    }
}

IUIAutomationElement* CorrectLayout::getFocusedElement()
{
    IUIAutomationElement *element;

    HRESULT hr = _automation->GetFocusedElement(&element);
    if(FAILED(hr))
        return NULL;

    return element;
}

QString CorrectLayout::getElementText(IUIAutomationElement* element)
{
    if(element == NULL)
        return "";

    VARIANT name;
    HRESULT hr = element->GetCurrentPropertyValue(UIA_ValueValuePropertyId ,&name);

    if(SUCCEEDED(hr))
    {
        wchar_t* t = name.bstrVal;
        QString s = QString::fromWCharArray(t);
        return s;
    }

    return "";
}
bool CorrectLayout::compareElements(IUIAutomationElement *element1, IUIAutomationElement *element2)
{
    if(element1 == NULL || element2 == NULL)
        return false;

    VARIANT runtimeID1;
    HRESULT hr1 = element1->GetCurrentPropertyValue(UIA_RuntimeIdPropertyId ,&runtimeID1);
    SAFEARRAY* sa1 = runtimeID1.parray;

    LONG lBound1, uBound1;
    SafeArrayGetLBound(sa1, 1, &lBound1);
    SafeArrayGetUBound(sa1, 1, &uBound1);
    LONG count1 = uBound1 - lBound1 + 1;  // bounds are inclusive

    int* raw1;
    SafeArrayAccessData(sa1, (void**)&raw1);

    std::vector<int> v1(raw1, raw1 + count1);

    VARIANT runtimeID2;
    HRESULT hr2 = element2->GetCurrentPropertyValue(UIA_RuntimeIdPropertyId ,&runtimeID2);
    SAFEARRAY* sa2 = runtimeID2.parray;

    LONG lBound2, uBound2;
    SafeArrayGetLBound(sa2, 1, &lBound2);
    SafeArrayGetUBound(sa2, 1, &uBound2);
    LONG count2 = uBound2 - lBound2 + 1;  // bounds are inclusive

    int* raw2;
    SafeArrayAccessData(sa2, (void**)&raw2);

    std::vector<int> v2(raw2, raw2 + count2);

    if(v1.size() == v2.size())
    {
        for(int i = 0; i < v1.size(); i++)
        {
            if(v1[i] != v2[i])
                return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}
