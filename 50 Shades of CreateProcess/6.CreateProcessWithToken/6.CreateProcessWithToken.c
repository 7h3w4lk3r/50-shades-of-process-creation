/*   -------------------------------
     RUN THIS AWITH ADMIN PRIVILEGES
     -------------------------------
*/

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "advapi32.lib")

int main(void)
{
    HANDLE hToken = NULL;
    HANDLE hPrimaryToken = NULL;
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    // 1. Get the current process token (your own token)
    if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_DUPLICATE | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY,
        &hToken))
    {
        wprintf(L"OpenProcessToken failed: %lu\n", GetLastError());
        return 1;
    }

    // 2. Duplicate it as a primary token (required for CreateProcessWithTokenW)
    if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL,
        SecurityImpersonation, TokenPrimary, &hPrimaryToken))
    {
        wprintf(L"DuplicateTokenEx failed: %lu\n", GetLastError());
        CloseHandle(hToken);
        return 1;
    }

    // 3. Launch calc.exe with YOUR token
    //    dwLogonFlags = 0  -> no profile loading needed (it's already your profile)
    //    lpDesktop = "winsta0\\default" (optional, but good for GUI)
    si.lpDesktop = L"winsta0\\default";

    if (!CreateProcessWithTokenW(
        hPrimaryToken,
        0,                               // no profile loading
        L"C:\\Windows\\System32\\calc.exe",
        NULL,
        CREATE_NEW_CONSOLE,
        NULL,                            // use parent's environment
        L"C:\\Windows",
        &si,
        &pi))
    {
        wprintf(L"CreateProcessWithTokenW failed: %lu\n", GetLastError());
        CloseHandle(hPrimaryToken);
        CloseHandle(hToken);
        return 1;
    }

    wprintf(L"Process started (PID: %lu)\n", pi.dwProcessId);

    // Clean up
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(hPrimaryToken);
    CloseHandle(hToken);

    return 0;
}