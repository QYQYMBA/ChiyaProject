#include "correctlayout.h"

#include <QDebug>
#include <QCoreApplication>
#include <string>
#include <iostream>
#include <fstream>
#include <queue>

#include "dicitionariedownloader.h"
#include "winapiadapter.h"

const int MAXNAMELENGTH = 30;

CorrectLayout::CorrectLayout(HWND hwnd, LayoutController* layoutController)
    : timer(),
      _layoutChecker(layoutController)
{
    _settings.beginGroup("CorrectLayout");

    _running = false;
    _initialized = false;

    _layoutController = layoutController;
    _myHWND = hwnd;
    _changeLayout = 0x0;
    _oldLayout = 0x0;
    _position = -1;

    qDebug() << "Start loading dictionaries";
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

    qDebug() << "CorrectLayout initialization";

    if(!_initialized)
        init();

    if(!loadDictionaries())
    {
        qDebug() << "Can't load all dictionaries";
        return false;
    }

    qDebug() << "Initialization finished successfully";

    _keyPressId = QtGlobalInput::waitForKeyPress(0, EventType::ButtonUp, &CorrectLayout::handleKey, this, true);
    _mousePressId = QtGlobalInput::waitForMousePress(0, EventType::ButtonDown, &CorrectLayout::handleMouse, this, true);
    _keyLlPressId = QtGlobalInput::setLlKeyboardHook(0, EventType::ButtonDown, &CorrectLayout::handleLlKey, this, false);
    QtGlobalInput::removeLlKeyboardHook(_keyLlPressId);
    _windowSwitchId = QtGlobalInput::setWindowSwitch(&CorrectLayout::windowSwitched, this);

    _lastLayout = _layoutController->getLayout();

    _state = SwitcherState::SEARCHING;

    _running = true;

    qDebug() << "CorrectLayout started";

    return true;
}


bool CorrectLayout::stopCl()
{
    if(!_running)
        return false;

    _state = SwitcherState::STOPED;

    QtGlobalInput::removeKeyPress(_keyPressId);
    QtGlobalInput::removeMousePress(_mousePressId);
    QtGlobalInput::removeWindowSwitch(_windowSwitchId);

    _running = false;

    return true;
}

bool CorrectLayout::reinitialize()
{
    _layoutChecker = LayoutChecker(_layoutController);
    return loadDictionaries();
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

    _aeh = new AutomationEventHandle(_automation, this);
    _fceh = new FocusChangedEventHandler(_automation, _aeh, _layoutController, _passwordCapsLock, _passwordNewLayout);
    hr = _automation->AddFocusChangedEventHandler(NULL, (IUIAutomationFocusChangedEventHandler*)_fceh);
    if(FAILED(hr))
        qDebug() << "Focus changed event handler can't be added";

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

    QString windowClassName(WinApiAdapter::GetWindowClass(GetForegroundWindow()));
    if (windowClassName.contains("Qt") && windowClassName.contains("QWindow"))
    {
        _exception = true;
        return;
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

    if(_settings.value("shortcuts/shortcut/pause/active").toBool())
    {
        Key key;
        key.vkCode = _settings.value("shortcuts/shortcut/pause/vkCode").toInt();
        key.ctrl = _settings.value("shortcuts/shortcut/pause/ctrl").toBool();
        key.shift = _settings.value("shortcuts/shortcut/pause/shift").toBool();
        key.alt = _settings.value("shortcuts/shortcut/pause/alt").toBool();
        _shortcutPause = key;
    }
    else
    {
        Key key;
        key.vkCode = 0;
        _shortcutPause = key;
    }

    if(_settings.value("shortcuts/shortcut/next/active").toBool())
    {
        Key key;
        key.vkCode = _settings.value("shortcuts/shortcut/next/vkCode").toInt();
        key.ctrl = _settings.value("shortcuts/shortcut/next/ctrl").toBool();
        key.shift = _settings.value("shortcuts/shortcut/next/shift").toBool();
        key.alt = _settings.value("shortcuts/shortcut/next/alt").toBool();
        _shortcutNext = key;
    }
    else
    {
        Key key;
        key.vkCode = 0;
        _shortcutNext = key;
    }

    if(_settings.value("shortcuts/shortcut/undo/active").toBool())
    {
        Key key;
        key.vkCode = _settings.value("shortcuts/shortcut/undo/vkCode").toInt();
        key.ctrl = _settings.value("shortcuts/shortcut/undo/ctrl").toBool();
        key.shift = _settings.value("shortcuts/shortcut/undo/shift").toBool();
        key.alt = _settings.value("shortcuts/shortcut/undo/alt").toBool();
        _shortcutUndo = key;
    }
    else
    {
        Key key;
        key.vkCode = 0;
        _shortcutUndo = key;
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
            LayoutCorrectionSettings ls;

            QString layout = WinApiAdapter::hklToStr(layoutsList[i]);
            ls.active = !_settings.value("layouts/" +layout + "/deactivated").toBool();
            ls.check = _settings.value("layouts/" +layout + "/auto", true).toBool();


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

void CorrectLayout::convertSelection(HKL switchToLayout)
{
    QString selection = _aeh->getSelection();

    if(switchToLayout != 0)
    {
        _layoutController->switchLayout(switchToLayout);
    }
    else
    {
        HKL currentLayout = _layoutController->getLayout();
        int newLayout = 0;
        for(int i = 0; i < _layoutsSettings.size(); i++)
        {
            if(currentLayout == _layoutsSettings[i].layout)
            {
                newLayout = i + 1;
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
                _layoutController->switchLayout(_layoutsSettings[newLayout].layout);
                break;
            }
        }

    }

    _layoutChecker.changeWordLayout(selection, _layoutController->getLayout());
    WinApiAdapter::SendUnicodeString(selection);
    _state = SwitcherState::PAUSED;
}

QString CorrectLayout::convertCurrentWord(QString word)
{
    if (!_fceh->elementChanged(false))
    {
        _oldLayout = _layoutController->getLayout();
        _layoutChecker.changeWordLayout(word, _changeLayout);
        WinApiAdapter::ReplaceUnicodeString(word);
    }
    return word;
}

void CorrectLayout::checkLayout(const bool finished)
{
    if (_state == SwitcherState::WORKING) {
        QString wordToCheck = _currentWord;
        HKL layout = _layoutChecker.checkLayout(wordToCheck, false, _lastLayout);
        if (layout != nullptr && layout != _lastLayout) {
            int start = _position - _currentWord.size();
            int end = _currentText.indexOf(' ', start);
            if (end == -1)
                end = _currentText.size();
            QString changedWord = _currentText.mid(start, end-start);
            _layoutChecker.changeWordLayout(changedWord, layout);

            if(changedWord != _currentWord)
            {
                _changeLayout = layout;
                _keyLlPressId = QtGlobalInput::setLlKeyboardHook(0, EventType::ButtonDown, &CorrectLayout::handleLlKey, this, false);
            }
        }
        else
        {
            _changeLayout = 0x0;
            QString wordToCheck = _currentWord;
            HKL layout = _layoutChecker.checkLayout(wordToCheck, true);
            if (layout != nullptr && layout != _lastLayout) {
                int start = _position - _currentWord.size();
                int end = _currentText.indexOf(' ', start);
                if (end == -1)
                    end = _currentText.size();
                QString changedWord = _currentText.mid(start, end-start);
                _layoutChecker.changeWordLayout(changedWord, layout);
                if(changedWord != _currentWord)
                {
                    _changeLayoutSpace = layout;
                    _keyLlPressId = QtGlobalInput::setLlKeyboardHook(0, EventType::ButtonDown, &CorrectLayout::handleLlKey, this, false);
                }
            }
            else
            {
                _changeLayoutSpace = 0x0;
            }
        }
    }
}

bool CorrectLayout::loadDictionaries()
{
    for (int i = 0; i < _layoutsList.size(); i++)
    {
        bool check = false;
        for (LayoutCorrectionSettings ls : _layoutsSettings)
        {
            if(ls.layout == _layoutsList[i])
                check = ls.check;
        }

        wchar_t name[MAXNAMELENGTH];
        LANGID language = (LANGID)(((UINT)_layoutsList[i]) & 0x0000FFFF);
        LCID locale = MAKELCID(language, SORT_DEFAULT);

        GetLocaleInfo(locale, LOCALE_SLANGUAGE, name, MAXNAMELENGTH);

        qDebug() << "Start loading " << QString::fromWCharArray(name) << "(" << WinApiAdapter::decToHex(_layoutsList[i]) << ") dictionary";

        QString path = QCoreApplication::applicationDirPath() + "/Dictionaries/" + WinApiAdapter::decToHex(_layoutsList[i]) + ".txt";
        if (!QFile::exists(path))
        {
            DicitionarieDownloader* downloader;
            downloader = new DicitionarieDownloader();
            downloader->getData(WinApiAdapter::decToHex(_layoutsList[i]) + ".txt");
            QTimer timer;
            timer.setSingleShot(true);
            QEventLoop loop;
            connect( downloader, &DicitionarieDownloader::onReady, &loop, &QEventLoop::quit);
            connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit );
            timer.start(30000);
            loop.exec();
            if(timer.isActive())
            {
                qDebug() << "Dictionary file downloaded!";
            }
            else
            {
                qDebug() << "Dictionary file download failed!";
                return false;
            }

        }
        if (QFile::exists(path))
        {
            if(!_layoutChecker.load(path, _layoutsList[i], check))
            {
                qDebug() << "Error while loading dictionarie!";
                return false;
            }
        }
        else
        {
            qDebug() << "No dictionary file found!";
        }
        qDebug() << "Finish loading " << QString::fromWCharArray(name) << " dictionary";
    }

    qDebug() << "Finish loading dictionaries";

    return true;
}

void CorrectLayout::handleValueChange(QString newText)
{
    qDebug() << newText;

    if(_state == SwitcherState::STOPED)
        return;

    if(_state == SwitcherState::CHANGING)
        return;

    if(_exception)
        return;

    bool elementChanged = _fceh->elementChanged(true);

    HKL newLayout = _layoutController->getLayout();
    if(newText.size() > 2 && !elementChanged && newLayout != _lastLayout)
    {
        _state = SwitcherState::PAUSED;
    }
    _lastLayout = newLayout;

    _changeLayout = 0x0;
    _changeLayoutSpace = 0x0;

    int positionInText = 0;
    bool t = true;
    newText = newText.replace('\n', ' ').replace('\r', ' ');
    if(newText == "" || elementChanged){
        _state = SwitcherState::SEARCHING;
        _currentText = newText;
        _position = -1;
        return;
    }

    if(_state == SwitcherState::PAUSED)
        return;

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
            _position = -1;
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
                    _position = -1;
                }
            }
        }
    }
    else
    {
        _state = SwitcherState::SEARCHING;
        _position = -1;
    }

    _currentText = newText;
}

void CorrectLayout::handleKey(RAWKEYBOARD keyboard)
{
    if(!_running)
        return;

    if(_exception)
        return;

    if(keyboard.Message != WM_KEYUP && keyboard.Message != WM_SYSKEYUP)
        return;

    bool ctrl = GetAsyncKeyState(VK_LCONTROL) || GetAsyncKeyState(VK_RCONTROL);
    bool shift = GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT);
    bool alt = GetAsyncKeyState(VK_LMENU);

    if(_shortcutPause.vkCode == keyboard.VKey && _shortcutPause.vkCode != 0)
        if(_shortcutPause.ctrl == ctrl)
            if(_shortcutPause.shift == shift)
                if(_shortcutPause.alt == alt)
                {
                    if(_state == SwitcherState::STOPED)
                        _state = SwitcherState::PAUSED;
                    else
                        _state = SwitcherState::STOPED;
                    return;
                }

    if(_state == SwitcherState::STOPED)
        return;

    if(_state == SwitcherState::CHANGING)
        return;

    if(_shortcutNext.vkCode == keyboard.VKey && _shortcutNext.vkCode != 0)
        if(_shortcutNext.ctrl == ctrl)
            if(_shortcutNext.shift == shift)
                if(_shortcutNext.alt == alt)
                {
                    convertSelection(0);
                    return;
                }

    if(_shortcutUndo.vkCode == keyboard.VKey && _shortcutUndo.vkCode != 0)
        if(_shortcutUndo.ctrl == ctrl)
            if(_shortcutUndo.shift == shift)
                if(_shortcutUndo.alt == alt)
                {
                    if(_state == SwitcherState::WORKING)
                    {
                        _changeLayout = _oldLayout;
                        convertCurrentWord(_currentWord);
                        _layoutController->switchLayout(_changeLayout);
                        _changeLayout = 0x0;
                        _state = SwitcherState::PAUSED;
                    }
                    return;
                }

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
                        return;
                    }

        if(_layoutsSettings[i].shortcutSelect.vkCode == keyboard.VKey && _layoutsSettings[i].shortcutSelect.vkCode != 0)
            if(_layoutsSettings[i].shortcutSelect.ctrl == ctrl)
                if(_layoutsSettings[i].shortcutSelect.shift == shift)
                    if(_layoutsSettings[i].shortcutSelect.alt == alt)
                    {
                        convertSelection(_layoutsSettings[i].layout);
                        return;
                    }
    }

    if(keyboard.VKey == VK_SPACE || keyboard.VKey == VK_ACCEPT || keyboard.VKey == VK_BACK || keyboard.VKey == VK_DELETE)
       _fceh->newElement();
}

bool CorrectLayout::handleLlKey(int nCode, WPARAM wParam, LPARAM lParam)
{
    PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT) lParam;
    QtGlobalInput::removeLlKeyboardHook(_keyLlPressId);

    SwitcherState oldState = _state;
    _state = SwitcherState::CHANGING;
    if (key != nullptr) {
        unsigned long vkCode = key->vkCode;

        if (vkCode == VK_SPACE)
        {
            if (!_fceh->elementChanged(false) && _changeLayoutSpace != 0x0)
            {
                HRESULT hr = _fceh->activateTextChangedHandler(false);
                if(FAILED(hr))
                    qDebug() << "Text changed event handler can't be removed";
                int start = _position - _currentWord.size();
                int end = _currentText.indexOf(' ', start);
                if (end == -1)
                    end = _currentText.size();
                QString changedWord = _currentText.mid(start, end-start);
                _changeLayout = _changeLayoutSpace;
                changedWord = convertCurrentWord(changedWord);
                _layoutController->switchLayout(_changeLayout);
                _lastLayout = _changeLayout;
                _currentText = _currentText.replace(start, changedWord.size(), changedWord);
                _currentWord = changedWord;

                _changeLayout = 0x0;
                _changeLayoutSpace = 0x0;
                _state = oldState;
                hr = _fceh->activateTextChangedHandler(true);
                if(FAILED(hr))
                    qDebug() << "Text changed event handler can't be added";
                return false;
            }
        }
        if(_layoutChecker.vkToChar(_changeLayout, vkCode, false) != NULL)
            if (!_fceh->elementChanged(false) && _changeLayout != 0x0)
            {
                HRESULT hr = _fceh->activateTextChangedHandler(false);
                if(FAILED(hr))
                    qDebug() << "Text changed event handler can't be removed";
                int start = _position - _currentWord.size();
                int end = _currentText.indexOf(' ', start);
                if (end == -1)
                    end = _currentText.size();
                QString changedWord = _currentText.mid(start, end-start);
                changedWord = convertCurrentWord(changedWord);
                _lastLayout = _changeLayout;
                _layoutController->switchLayout(_changeLayout);
                _currentText = _currentText.replace(start, changedWord.size(), changedWord);
                _currentWord = changedWord;
                hr = _fceh->activateTextChangedHandler(true);
                if(FAILED(hr))
                    qDebug() << "Text changed event handler can't be added";
            }
    }

    _state = oldState;

    _changeLayout = 0x0;
    _changeLayoutSpace = 0x0;
    return false;
}

void CorrectLayout::handleMouse(RAWMOUSE mouse)
{
    _changeLayout = 0x0;
    QtGlobalInput::removeLlKeyboardHook(_keyLlPressId);
}
