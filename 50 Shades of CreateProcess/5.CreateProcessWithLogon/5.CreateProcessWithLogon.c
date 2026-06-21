#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "advapi32.lib")

int main(void)
{
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    // Initialize structures
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);

    // Command to run as another user
    WCHAR cmdLine[] = L"cmd.exe /k whoami";

    // Create process using supplied credentials
    if (!CreateProcessWithLogonW(
        L"TestUser",                 // Username
        L".",                        // Local computer
        L"Password123!",             // Password
        LOGON_WITH_PROFILE,          // Load user profile
        NULL,                        // Application name
        cmdLine,                     // Command line
        CREATE_UNICODE_ENVIRONMENT,  // Unicode environment
        NULL,                        // Environment block
        NULL,                        // Current directory
        &si,                         // Startup info
        &pi))                        // Process info
    {
        DWORD err = GetLastError();

        wprintf(
            L"CreateProcessWithLogonW failed: %lu (0x%08lX)\n",
            err,
            err);

        return 1;
    }

    // Process created successfully
    wprintf(
        L"Process created successfully.\n"
        L"PID: %lu\n",
        pi.dwProcessId);

    // Wait for the process to exit
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Cleanup handles
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return 0;
}