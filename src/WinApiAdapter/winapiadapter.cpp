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

    qDebug() << exeName;
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

std::string WinApiAdapter::hklToStr(HKL hkl)
{
    std::stringstream s;
    s << hkl;
    return s.str().substr(8,8);
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
                list.push_back(layoutsList[j]);
                layoutsList[j] = 0;
                break;
            }
        }
    }

    for(int i = 0; i < n; i++)
    {
        if(layoutsList[i] != 0)
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
        PostMessage(foreground, WM_INPUTLANGCHANGEREQUEST, NULL, (LPARAM)layout);
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

WinApiAdapter::WinApiAdapter()
{
}
