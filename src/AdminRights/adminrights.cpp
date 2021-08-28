#include "adminrights.h"

#include <tchar.h>
#include <string>
#include <iostream>

BOOL AdminRights::IsRunAsAdministrator()
{
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = NULL;

    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pAdministratorsGroup))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Determine whether the SID of administrators group is enabled in
    // the primary access token of the process.
    if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (pAdministratorsGroup)
    {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError)
    {
        throw dwError;
    }

    return fIsRunAsAdmin;
}

void AdminRights::ElevateNow(LPCWSTR param)
{
    BOOL bAlreadyRunningAsAdministrator = FALSE;
    try
    {
        bAlreadyRunningAsAdministrator = AdminRights::IsRunAsAdministrator();
    }
    catch(...)
    {
        std::cout << "Failed to determine if application was running with admin rights" << std::endl;
        DWORD dwErrorCode = GetLastError();
        TCHAR szMessage[256];
        _stprintf_s(szMessage, ARRAYSIZE(szMessage), _T("Error code returned was 0x%08lx"),dwErrorCode);
        std::cout << szMessage << std::endl;
    }
    if(!bAlreadyRunningAsAdministrator)
    {
        wchar_t szPath[MAX_PATH];
        if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
        {
            // Launch itself as admin
            SHELLEXECUTEINFO sei = { sizeof(sei) };
            sei.lpVerb = L"runas";
            sei.lpFile = szPath;
            sei.lpParameters = param;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;

            if (!ShellExecuteEx(&sei))
            {
                DWORD dwError = GetLastError();
                if (dwError == ERROR_CANCELLED)
                {
                    // The user refused to allow privileges elevation.
                    std::cout << "End user did not allow elevation" << std::endl;
                }
            }
            else
            {
                _exit(1);  // Quit itself
            }
        }
    }
}
