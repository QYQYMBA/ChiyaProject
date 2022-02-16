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
    : _llKeyHandler(),
      timer(),
      _layoutChecker(),
      _queueMutex()
{
    _settings.beginGroup("CorrectLayout");

    _running = false;
    _initialized = false;

    _layoutController = layoutController;
    _currentElement = NULL;

    _myHWND = hwnd;
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

    if(!_initialized)
    {
        if(!init())
            return false;
    }

    _lastLayout = _layoutController->getLayout();

    getLayoutSettingsList();

    loadSettings();

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
    HRESULT hr = CoCreateInstance(__uuidof(CUIAutomation), NULL,
                                  CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation),
                                  (void**)&_automation);

    if(FAILED(hr))
        qDebug() << "Automation can not be initialized";

    //fceh = new FocusChangedEventHandler(this, _automation);
    //_automation->AddFocusChangedEventHandler(NULL, (IUIAutomationFocusChangedEventHandler*)fceh);

    qDebug() << "Start loading dictionaries";

    std::vector<HKL> layoutsList = WinApiAdapter::getLayoutsList();

    _keyPressId = QtGlobalInput::waitForKeyPress(0, EventType::ButtonDown, &CorrectLayout::handleKey, this, true);
     _keyLlPressId = QtGlobalInput::setLlKeyboardHook(0, EventType::ButtonDown, &CorrectLayout::handleLlKey, this, false);
    _mousePressId = QtGlobalInput::waitForMousePress(0, EventType::ButtonDown, &CorrectLayout::handleMouse, this, true);
    _windowSwitchId = QtGlobalInput::setWindowSwitch(&CorrectLayout::windowSwitched, this);

    for (int i = 0; i < layoutsList.size(); i++)
    {
        wchar_t name[MAXNAMELENGTH];
        LANGID language = (LANGID)(((UINT)layoutsList[i]) & 0x0000FFFF);
        LCID locale = MAKELCID(language, SORT_DEFAULT);

        GetLocaleInfo(locale, LOCALE_SLANGUAGE, name, MAXNAMELENGTH);

        qDebug() << "Start loading " << QString::fromWCharArray(name) << " dictionary";

        QString path = QCoreApplication::applicationDirPath() + "/Dictionaries/" + WinApiAdapter::decToHex(layoutsList[i]) + ".txt";
        _layoutChecker.load(path, layoutsList[i]);
        qDebug() << "Finish loading " << QString::fromWCharArray(name) << " dictionary";
    }

    qDebug() << "Finish loading dictionaries";

    _initialized = true;

    connect(this, &CorrectLayout::finished, this, &CorrectLayout::onKeyProcessed);

    qDebug() << "Initialization finished successfully";

    return true;
}

bool CorrectLayout::isRunning()
{
    return _running;
}

void CorrectLayout::handleKey(RAWKEYBOARD rk)
{
    if(_state == SwitcherState::CHANGING)
    {
        return;
    }

    bool ctrl = GetAsyncKeyState(VK_CONTROL);
    bool shift = GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT);
    bool alt = GetAsyncKeyState(VK_MENU);

    KeyPress keyPress(rk, shift, true, ctrl, alt, _layoutController->getLayout());

    addKeyToQueue(keyPress);
}

bool CorrectLayout::handleLlKey(int nCode, WPARAM wParam, LPARAM lParam)
{
    _llKeyHandler.passArguments(nCode, wParam, lParam);
    _llKeyHandler.start();
    QDeadlineTimer dt(5);
    _llKeyHandler.wait(dt);
    return _llKeyHandler.getReslut();
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

    std::queue<KeyPress> keys;
    if(_keyPresses.size() > _currentWord.size())
        for(int i = _keyPresses.size() - _currentWord.size(); i < _keyPresses.size(); i++)
        {
            keys.push(_keyPresses[i]);
        }

    if (layout == NULL) {

    }
    else {
        _layoutController->switchLayout(layout);
    }

    QVector<KeyPress> keyPresses;
    keyPresses.push_back(KeyPress(VK_SCROLL));
    for(int i = 0; i < keys.size(); i++)
    {
         keyPresses.push_back(KeyPress(VK_BACK));
    }

    while (!keys.empty()) {
        keyPresses.push_back(keys.front());
        keys.pop();
    }

    keyPresses.push_back(KeyPress(VK_PAUSE));

    _state = SwitcherState::CHANGING;

    WinApiAdapter::SendKeyPresses(keyPresses);

    _state = SwitcherState::WORKING;

    _lastLayout = _layoutController->getLayout();
}

void CorrectLayout::checkLayout(const bool beforeKeyPress, const bool finished)
{
    if (_state == SwitcherState::WORKING) {
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

void CorrectLayout::addKeyToQueue(KeyPress kp)
{
    _queueMutex.lock();
    _keyQueue.enqueue(kp);
    _queueMutex.unlock();
    this->start();
}

IUIAutomationElement* CorrectLayout::getFocusedElement()
{
    IUIAutomationElement *element;

    HRESULT hr = _automation->GetFocusedElement(&element);
    if(FAILED(hr))
    {
        qDebug() << "Failed to get focused element.";
        return NULL;
    }

    return element;
}

QString CorrectLayout::getElementText(IUIAutomationElement* element)
{
    if(element == NULL)
    {
        return "";
    }

    IUIAutomationCondition* textPatternCondition;
    IUIAutomationElement* keyboardFocus = NULL;
    VARIANT trueVar;
    trueVar.vt = VT_BOOL;
    trueVar.boolVal = VARIANT_TRUE;
    HRESULT hr = _automation->CreatePropertyCondition(UIA_HasKeyboardFocusPropertyId, trueVar, &textPatternCondition);

    if (FAILED(hr))
    {
        qDebug() << "Failed to CreatePropertyCondition";
        return "";
    }
    else
    {
        hr = element->FindFirst(TreeScope_Subtree, textPatternCondition, &keyboardFocus);
        textPatternCondition->Release();
        if (FAILED(hr))
        {
            qDebug() << "FindFirst failed";
        }
        else if (keyboardFocus == NULL)
        {
            qDebug() << "No element with keyboard focus found.";
            return "";
        }
    }
    VARIANT text;
    hr = keyboardFocus->GetCurrentPropertyValue(UIA_ValueValuePropertyId ,&text);
    keyboardFocus->Release();
    if (FAILED(hr))
    {
        qDebug() << "Failed to get element value.";
        return "";
    }
    if(SUCCEEDED(hr))
    {
        wchar_t* t = text.bstrVal;
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

void CorrectLayout::onKeyProcessed()
{
    //timer.stop();
    _queueMutex.lock();
    if(!_keyQueue.empty())
    {
        _keyQueue.dequeue();
        this->start();
    }
    _queueMutex.unlock();
}

void CorrectLayout::run()
{
    if(!_running)
        return;

    if (!GetAsyncKeyState(VK_LSHIFT) && !GetAsyncKeyState(VK_RSHIFT))
    {
        INPUT input = WinApiAdapter::MakeKeyInput(VK_SHIFT, false);
        SendInput(1, &input, sizeof(INPUT));
    }

    if (_state == SwitcherState::CHANGING)
        return;

    if(_exception)
        return;

    //timer.start(10);

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
    KeyPress keyPress;
    if(_queueMutex.try_lock_for(std::chrono::milliseconds(10)) &&_keyQueue.size() > 0)
    {
        keyPress = _keyQueue.head();
        _queueMutex.unlock();
    }
    else
    {
        _queueMutex.unlock();
        return;
    }

    IUIAutomationElement* newElement = getFocusedElement();
    if(_currentElement == NULL || !compareElements(_currentElement, newElement))
    {
        _currentText = "";
        if(_currentElement != NULL)
            _currentElement->Release();
        _currentElement = newElement;
        _state = SwitcherState::SEARCHING;
        _keyPresses.clear();
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

    HKL newLayout = _layoutController->getLayout();
    if(newLayout != _lastLayout && _lastLayout != NULL)
    {
        _state = SwitcherState::PAUSED;
        _lastLayout = newLayout;
    }

    _currentText = _currentText.replace('\n', ' ').replace('\r', ' ');

    if(_state == SwitcherState::PAUSED)
        return;

    if(_keyPresses.size() == 0)
    {
        _keyPresses.push_back(keyPress);
        return;
    }

    if(keyPress.getVkCode() == VK_BACK)
    {
        _keyPresses.pop_back();
        _currentText = getElementText(_currentElement).replace('\n', ' ').replace('\r', ' ');
        return;
    }

    if(!keyPress.isPrintable())
    {
        _keyPresses.push_back(keyPress);
        return;
    }

    _keyPresses.push_back(keyPress);

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
                    for(int i = oldWord.size(); i >= 0; i--)
                    {
                        QString variant = oldWord;

                        variant.insert(i, keyPress.toChar());

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
                    oldWord.insert(_position, keyPress.toChar());
                    oldWord = oldWord.toLower();
                    QString newWord = changedWords[1].toLower();
                    if(newWord == oldWord)
                    {
                        _position++;

                        _currentWord = newWord;
                        if (_currentWord.length() > 2) {
                            bool needShift = GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT);
                            checkLayout(false, false);
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
    }
    else
    {
        _state = SwitcherState::PAUSED;
    }
}
