#include "winapiadapter.h"

#include <QDebug>
#include <QSettings>

#include <sstream>
#include "Psapi.h"
#include "Winbase.h"

const int MAXNAMELENGTH = 30;

QString WinApiAdapter::GetWindowPath(HWND hwnd)
{
    TCHAR buffer[MAX_PATH];
    DWORD cchLen = MAX_PATH;
    if (hwnd)
    {
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (hProcess)
        {
            BOOL ret = QueryFullProcessImageName(hProcess, 0, buffer, &cchLen);

            CloseHandle(hProcess);
            if(!ret)
                return "";
        }
    }

    QString s;
    s = s.fromWCharArray(buffer);
    return s;
}

QString WinApiAdapter::GetWindowExeName(HWND hwnd)
{
    QString path = GetWindowPath(hwnd);
    int lastSlash = path.lastIndexOf('\\');
    QString exeName = path.mid(lastSlash + 1, path.length() - lastSlash);

    return exeName;
}

QString WinApiAdapter::GetWindowName(HWND hwnd)
{
    WCHAR buffer[256];
    GetWindowText(hwnd, buffer, 20);
    char ch[260];
    char DefChar = ' ';
    WideCharToMultiByte(CP_ACP, 0, buffer, -1, ch, 260, &DefChar, NULL);
    QString className(ch);

    return className;
}

QString WinApiAdapter::GetWindowClass(HWND hwnd)
{
    WCHAR buffer[256];
    GetClassNameW(hwnd, buffer, 20);
    char ch[260];
    char DefChar = ' ';
    WideCharToMultiByte(CP_ACP, 0, buffer, -1, ch, 260, &DefChar, NULL);
    QString className(ch);

    return className;
}

QString WinApiAdapter::hklToStr(HKL hkl)
{
    std::stringstream s;
    s << hkl;
    std::string ss = s.str().substr(8,8);
    return QString::fromStdString(ss);
}

QString WinApiAdapter::vkToString(uint vkCode)
{
    if (vkCode == VK_PAUSE)
        return "Break";
    if (vkCode == VK_SNAPSHOT)
        return "PRTSC";
    if (vkCode == VK_SCROLL)
        return "SCRLK";
    if (vkCode == VK_INSERT)
        return "Insert";
    if (vkCode == VK_HOME)
        return "Home";
    if (vkCode == VK_PRIOR)
        return "PG UP";
    if (vkCode == VK_NEXT)
        return "PG DN";
    if (vkCode == VK_END)
        return "End";
    if (vkCode == VK_DELETE)
        return "Del";
    if (vkCode == VK_UP)
        return "Up";
    if (vkCode == VK_DOWN)
        return "Down";
    if (vkCode == VK_LEFT)
        return "Left";
    if (vkCode == VK_RIGHT)
        return "Right";
    if (vkCode == VK_F1)
        return "F1";
    if (vkCode == VK_F2)
        return "F2";
    if (vkCode == VK_F3)
        return "F3";
    if (vkCode == VK_F4)
        return "F4";
    if (vkCode == VK_F5)
        return "F5";
    if (vkCode == VK_F6)
        return "F6";
    if (vkCode == VK_F7)
        return "F7";
    if (vkCode == VK_F8)
        return "F8";
    if (vkCode == VK_F9)
        return "F9";
    if (vkCode == VK_F10)
        return "F10";
    if (vkCode == VK_F11)
        return "F11";
    if (vkCode == VK_F12)
        return "F12";
    if (vkCode == VK_ESCAPE)
        return "ESC";
    if (vkCode == VK_NUMLOCK)
        return "Numlock";
    if (vkCode == VK_CAPITAL)
        return "Capslock";
    std::string s = "";
    s += MapVirtualKey(vkCode, MAPVK_VK_TO_CHAR);
    return QString::fromStdString(s);
}

std::vector<HKL> WinApiAdapter::getLayoutsList()
{
    UINT n;
    HKL* layoutsList = NULL;

    n = GetKeyboardLayoutList(0, NULL);
    layoutsList = (HKL*)LocalAlloc(LPTR, (n * sizeof(HKL)));
    n = GetKeyboardLayoutList(n, layoutsList);

    QSettings systemSettings("HKEY_CURRENT_USER\\Control Panel\\International\\User Profile", QSettings::Registry64Format);
    QStringList order = systemSettings.value("Languages").toStringList();

    std::vector<HKL> list;

    for(int i = 0; i < order.size(); i++)
    {
        for(int j = 0; j < n; j++)
        {
            wchar_t name[MAXNAMELENGTH];
            LANGID language = (LANGID)(((UINT)layoutsList[j]) & 0x0000FFFF);
            LCID locale = MAKELCID(language, SORT_DEFAULT);

            GetLocaleInfo(locale, LOCALE_SNAME, name, MAXNAMELENGTH);

            if(!order[i].contains('-'))
            {
                for(int i = 0; i < MAXNAMELENGTH; i++)
                {
                    if(name[i] == '-')
                    {
                        name[i] = '\0';
                        break;
                    }
                }
            }

            if(QString::fromWCharArray(name) == order[i])
            {
                if (layoutsList[j] != 0x0)
                {
                    list.push_back(layoutsList[j]);
                    layoutsList[j] = 0x0;
                    break;
                }
            }
        }
    }

    for(int i = 0; i < n; i++)
    {
        if(layoutsList[i] != 0x0)
        {
            list.push_back(layoutsList[i]);
        }
    }

    return list;
}

void WinApiAdapter::SetKeyboardLayout(HKL layout) {
    HWND foreground = GetForegroundWindow();
    HWND owner = GetWindow(foreground, GW_OWNER);
    if(owner != NULL)
    {
        HWND prevOwner;
        do
        {
            prevOwner = owner;
            owner = GetWindow(owner, GW_OWNER);
        } while(owner != NULL);
        PostMessage(prevOwner, WM_INPUTLANGCHANGEREQUEST, NULL, (LPARAM)layout);
    }
    else
    {
        PostMessage(foreground, WM_INPUTLANGCHANGEREQUEST, NULL, (LPARAM)layout);
    }
}

INPUT WinApiAdapter::MakeKeyInput(int vkCode, bool down)
{
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    ip.ki.wVk = vkCode;
    ip.ki.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
    return ip;
}

HKL WinApiAdapter::GetLayoutByHwnd(HWND hwnd) {
    return GetKeyboardLayout(GetWindowThreadProcessId(hwnd, NULL));
}

HKL WinApiAdapter::GetLayout() {
    return GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL));
}

void WinApiAdapter::SendKeyPress(int vkCode, bool shift, bool ctrl, bool alt)
{
    INPUT down = MakeKeyInput(vkCode, true);
    INPUT up = MakeKeyInput(vkCode, false);

    if(ctrl)
    {
        INPUT ctrlDown = MakeKeyInput(VK_CONTROL, true);
        SendInput(1, &ctrlDown, sizeof(INPUT));
    }

    if(shift)
    {
        INPUT shiftDown = MakeKeyInput(VK_SHIFT, true);
        SendInput(1, &shiftDown, sizeof(INPUT));
    }

    if(alt)
    {
        INPUT altDown = MakeKeyInput(VK_MENU, true);
        SendInput(1, &altDown, sizeof(INPUT));
    }

    SendInput(1, &down, sizeof(INPUT));
    SendInput(1, &up, sizeof(INPUT));

    if(ctrl)
    {
        INPUT ctrlUp = MakeKeyInput(VK_CONTROL, false);
        SendInput(1, &ctrlUp, sizeof(INPUT));
    }

    if(shift)
    {
        INPUT shiftUp = MakeKeyInput(VK_SHIFT, false);
        SendInput(1, &shiftUp, sizeof(INPUT));
    }

    if(alt)
    {
        INPUT altUp = MakeKeyInput(VK_MENU, false);
        SendInput(1, &altUp, sizeof(INPUT));
    }

}

void WinApiAdapter::ReplaceUnicodeString(QString s)
{
    INPUT* inputs = new INPUT[s.size()*4];
    int i = 0;
    for (QChar c : s)
    {
        inputs[i].type = INPUT_KEYBOARD;
        inputs[i].ki.wScan = 0;
        inputs[i].ki.time = 0;
        inputs[i].ki.dwExtraInfo = 0;

        inputs[i].ki.wVk = VK_BACK;
        inputs[i].ki.dwFlags = 0;
        i++;

        inputs[i] = inputs[i-1];
        inputs[i].ki.dwFlags |= KEYEVENTF_KEYUP;
        i++;
    }
    for (QChar c : s)
    {
        inputs[i].type = INPUT_KEYBOARD;
        inputs[i].ki.wScan = c.unicode();
        inputs[i].ki.time = 0;
        inputs[i].ki.dwExtraInfo = 0;

        inputs[i].ki.wVk = 0;
        inputs[i].ki.dwFlags = KEYEVENTF_UNICODE;
        i++;

        inputs[i] = inputs[i-1];
        inputs[i].ki.dwFlags |= KEYEVENTF_KEYUP;
        i++;
    }

    SendInput(s.size()*4, inputs, sizeof(INPUT));
    delete[] inputs;
}

void WinApiAdapter::SendUnicodeString(QString s)
{
    INPUT* inputs = new INPUT[s.size()*2];
    int i = 0;
    for (QChar c : s)
    {
        inputs[i].type = INPUT_KEYBOARD;
        inputs[i].ki.wScan = c.unicode();
        inputs[i].ki.time = 0;
        inputs[i].ki.dwExtraInfo = 0;

        inputs[i].ki.wVk = 0;
        inputs[i].ki.dwFlags = KEYEVENTF_UNICODE;
        i++;

        inputs[i] = inputs[i-1];
        inputs[i].ki.dwFlags |= KEYEVENTF_KEYUP;
        i++;
    }

    SendInput(s.size()*2, inputs, sizeof(INPUT));
    delete[] inputs;
}

void WinApiAdapter::SendUnicodeChar(QChar c)
{
    INPUT* inputs = new INPUT[2];
    int i = 0;

    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wScan = c.unicode();
    inputs[i].ki.time = 0;
    inputs[i].ki.dwExtraInfo = 0;

    inputs[i].ki.wVk = 0;
    inputs[i].ki.dwFlags = KEYEVENTF_UNICODE;
    i++;

    inputs[i] = inputs[i-1];
    inputs[i].ki.dwFlags |= KEYEVENTF_KEYUP;
    i++;

    SendInput(2, inputs, sizeof(INPUT));
    delete[] inputs;
}

void WinApiAdapter::SendKeyPress(KeyPress kp)
{
    INPUT down = MakeKeyInput(kp.getVkCode(), true);
    INPUT up = MakeKeyInput(kp.getVkCode(), false);

    if(kp.isCtrlPressed())
    {
        INPUT ctrlDown = MakeKeyInput(VK_CONTROL, true);
        SendInput(1, &ctrlDown, sizeof(INPUT));
    }

    if(kp.isShiftPressed())
    {
        INPUT shiftDown = MakeKeyInput(VK_SHIFT, true);
        SendInput(1, &shiftDown, sizeof(INPUT));
    }

    if(kp.isAltPressed())
    {
        INPUT altDown = MakeKeyInput(VK_MENU, true);
        SendInput(1, &altDown, sizeof(INPUT));
    }

    SendInput(1, &down, sizeof(INPUT));
    SendInput(1, &up, sizeof(INPUT));

    if(kp.isCtrlPressed())
    {
        INPUT ctrlUp = MakeKeyInput(VK_CONTROL, false);
        SendInput(1, &ctrlUp, sizeof(INPUT));
    }

    if(kp.isShiftPressed())
    {
        INPUT shiftUp = MakeKeyInput(VK_SHIFT, false);
        SendInput(1, &shiftUp, sizeof(INPUT));
    }

    if(kp.isAltPressed())
    {
        INPUT altUp = MakeKeyInput(VK_MENU, false);
        SendInput(1, &altUp, sizeof(INPUT));
    }
}

void WinApiAdapter::SendKeyPresses(QVector<KeyPress> keyPresses)
{
    INPUT pInputs[1000];
    UINT n = 0;
    for(KeyPress kp : keyPresses)
    {

        if(kp.isCtrlPressed())
        {
            pInputs[++n] = MakeKeyInput(VK_CONTROL, true);
        }

        if(kp.isShiftPressed())
        {
            pInputs[++n] = MakeKeyInput(VK_SHIFT, true);
        }

        if(kp.isAltPressed())
        {
            pInputs[++n] = MakeKeyInput(VK_MENU, true);
        }

        pInputs[++n] = MakeKeyInput(kp.getVkCode(), true);
        pInputs[++n] = MakeKeyInput(kp.getVkCode(), false);

        if(kp.isCtrlPressed())
        {
            pInputs[++n] = MakeKeyInput(VK_CONTROL, false);
        }

        if(kp.isShiftPressed())
        {
            pInputs[++n] = MakeKeyInput(VK_SHIFT, false);
        }

        if(kp.isAltPressed())
        {
            pInputs[++n] = MakeKeyInput(VK_MENU, false);
        }
    }
    SendInput(n, pInputs, sizeof(INPUT));
}

WinApiAdapter::WinApiAdapter()
{
}
