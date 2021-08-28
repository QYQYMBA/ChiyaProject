#ifndef ADMINRIGHTS_H
#define ADMINRIGHTS_H

#include "windows.h"

typedef PKBDLLHOOKSTRUCT KBDHOOK;

class AdminRights
{
public:
    static BOOL IsRunAsAdministrator();
    static void ElevateNow(LPCWSTR param);
private:
    AdminRights();
};

#endif // ADMINRIGHTS_H
