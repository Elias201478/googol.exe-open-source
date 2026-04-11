#pragma once
#include <windows.h>

typedef NTSTATUS(NTAPI* NtShutdownSystem_t)(ULONG);

static void EnableShutdownPrivilege() {
    HANDLE t;
    TOKEN_PRIVILEGES p;
    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &t);
    LookupPrivilegeValueA(0, "SeShutdownPrivilege", &p.Privileges[0].Luid);
    p.PrivilegeCount = 1;
    p.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(t, 0, &p, 0, 0, 0);
    CloseHandle(t);
}

static void Shutdown(bool Force) {
    EnableShutdownPrivilege();
    NtShutdownSystem_t f = (NtShutdownSystem_t)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtShutdownSystem");
    if (!f) return;
    f(Force ? 1 : 0);
}