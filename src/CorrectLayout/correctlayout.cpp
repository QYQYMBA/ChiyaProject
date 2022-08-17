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

    _pceh = new PropertyChangedEventHandler(_automation, this);
    _fceh = new FocusChangedEventHandler(_automation, _pceh);
    hr = _automation->AddFocusChangedEventHandler(NULL, (IUIAutomationFocusChangedEventHandler*)_fceh);

    qDebug() << "Start loading dictionaries";

    std::vector<HKL> layoutsList = WinApiAdapter::getLayoutsList();

    _windowSwitchId = QtGlobalInput::setWindowSwitch(&CorrectLayout::windowSwitched, this);

    for (int i = 0; i < layoutsList.size(); i++)
    {
        wchar_t name[MAXNAMELENGTH];
        LANGID language = (LANGID)(((UINT)layoutsList[i]) & 0x0000FFFF);
        LCID locale = MAKELCID(language, SORT_DEFAULT);

        GetLocaleInfo(locale, LOCALE_SLANGUAGE, name, MAXNAMELENGTH);

        qDebug() << "Start loading " << QString::fromWCharArray(name) << " dictionary";

        QString path = QCoreApplication::applicationDirPath() + "/Dictionaries/" + WinApiAdapter::decToHex(layoutsList[i]) + ".txt";
        if (QFile::exists(path))
            _layoutChecker.load(path, layoutsList[i]);
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

void CorrectLayout::convertCurrentWord(HKL layout, bool finised)
{
    qDebug() << "Nado";
}

void CorrectLayout::checkLayout(const bool finished)
{
    if (_state == SwitcherState::WORKING) {
        QString wordToCheck = _currentWord;
        HKL layout = _layoutChecker.checkLayout(wordToCheck, finished);
        if (layout != nullptr && layout != _lastLayout) {
            QString changedWord = wordToCheck;
            _layoutChecker.changeWordLayout(changedWord, layout);
            if(changedWord != _currentWord)
            {
                convertCurrentWord(layout, finished);
                _currentWord = changedWord;
            }

        }
    }
}

/*void CorrectLayout::handleKeyAsync()
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
    if(_queueMutex.try_lock_for(std::chrono::milliseconds(10)) &&_keyQueue.size() > 0)
    {
        _keyToProcess = _keyQueue.head();
        _queueMutex.unlock();
    }
    else
    {
        _queueMutex.unlock();
        return;
    }

    IUIAutomationElement* newElement = getFocusedElement();
    _currentElement = getFocusedElement();
    if(_currentElement == NULL || !compareElements(_currentElement, newElement))
    {
        if(_currentElement != NULL)
            _currentElement->Release();
        _currentElement = newElement;
        _state = SwitcherState::SEARCHING;
        _keyPresses.clear();
        _currentText = getElementText(_currentElement);
    }
    if(_fceh->elementChanged(true))
    {
        _state = SwitcherState::SEARCHING;
        _keyPresses.clear();
        _currentText = _pceh->getText();
    }

    if(_keyToProcess.getVkCode() == VK_RETURN)
    {
        _state = SwitcherState::SEARCHING;
        _keyPresses.clear();
        _currentText = _pceh->getText();
        return;
    }

    if(_keyToProcess.getVkCode() == VK_SPACE)
    {
        checkLayout(false, true);
        _state = SwitcherState::SEARCHING;
        _keyPresses.clear();
        _currentText = _pceh->getText();
        return;
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

    if(_keyToProcess.getVkCode() == VK_BACK)
    {
        if(!_keyPresses.empty())
        {
            _keyPresses.pop_back();
        }
        _currentText = _pceh->getText().replace('\n', ' ').replace('\r', ' ');
        return;
    }

    if(!_keyToProcess.isPrintable())
    {
        _keyPresses.push_back(_keyToProcess);
        _currentText = _pceh->getText();
        return;
    }

    _keyPresses.push_back(_keyToProcess);

    if(_layoutChecker.isKeyInDictionary(_keyToProcess.getVkCode()))
    {
        QString newText = _pceh->getText().replace('\n', ' ').replace('\r', ' ');
        qDebug() << newText;
        qDebug() << _currentText;
        qDebug() << "\n\n\n\n\n";
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

                        variant.insert(i, _keyToProcess.toChar());

                        if(variant.toLower() == newWord.toLower() && i == newWord.size() - 1)
                        {
                            _position = i + 1;
                            _state = SwitcherState::WORKING;
                        }
                    }
                }
                else if(_state == SwitcherState::WORKING)
                {
                    QString oldWord = changedWords[0];
                    oldWord.insert(_position, _keyToProcess.toChar());
                    QString newWord = changedWords[1];
                    if(newWord.toLower() == oldWord.toLower())
                    {
                        _position++;

                        _currentWord = newWord;
                        if (_currentWord.length() > 2) {
                            bool needShift = GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT);
                            qDebug() << _currentWord;
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
        if(_keyToProcess.getVkCode() != VK_SHIFT && _keyToProcess.getVkCode() != VK_LSHIFT && _keyToProcess.getVkCode() != VK_RSHIFT && _keyToProcess.getVkCode() != VK_CAPITAL)
            _state = SwitcherState::PAUSED;
    }
}*/

void CorrectLayout::handleValueChange(QString newText)
{
    HKL newLayout = _layoutController->getLayout();
    if(newLayout != _lastLayout)
    {
        _state = SwitcherState::PAUSED;
        _lastLayout = newLayout;
    }

    newText = newText.replace('\n', ' ').replace('\r', ' ');
    if(!_fceh->elementChanged(true)){
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
                    if(newWord.mid(0, newWord.length()-1) == oldWord)
                    {
                        _position = newWord.length();
                        _state = SwitcherState::WORKING;
                    }
                }
                else if(_state == SwitcherState::WORKING)
                {
                    QString oldWord = changedWords[0];
                    QString newWord = changedWords[1];
                    if(newWord.mid(0, newWord.length()-1) == oldWord)
                    {
                        _position++;

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
    }
    else
    {
        _state = SwitcherState::SEARCHING;
    }
    _currentText = newText;
}
