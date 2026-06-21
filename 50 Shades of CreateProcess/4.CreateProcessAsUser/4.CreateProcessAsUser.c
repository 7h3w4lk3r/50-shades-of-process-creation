#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE

#include <windows.h>
#include <userenv.h>
#include <stdio.h>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "userenv.lib")

static BOOL EnablePrivilege(LPCWSTR privilege)
{
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return FALSE;

    if (!LookupPrivilegeValueW(NULL, privilege, &luid))
    {
        CloseHandle(hToken);
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL))
    {
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);
    return (GetLastError() == ERROR_SUCCESS);
}

int main(void)
{
    HANDLE hToken = NULL;
    HANDLE hPrimaryToken = NULL;
    HANDLE hProfile = NULL;   // Handle to the loaded profile

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    LPVOID lpEnvironment = NULL;
    DWORD exitCode = 0;

    EnablePrivilege(SE_IMPERSONATE_NAME);
    EnablePrivilege(SE_RESTORE_NAME);        // Required for LoadUserProfile
    EnablePrivilege(SE_BACKUP_NAME);         // Required for LoadUserProfile

    // 1. Logon user
    if (!LogonUserW(L"TestUser", L".", L"Password123!", LOGON32_LOGON_INTERACTIVE,
        LOGON32_PROVIDER_DEFAULT, &hToken))
    {
        wprintf(L"LogonUserW failed: %lu\n", GetLastError());
        return 1;
    }
    wprintf(L"[+] LogonUserW succeeded\n");

    // 2. Create primary token
    if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL,
        SecurityImpersonation, TokenPrimary, &hPrimaryToken))
    {
        wprintf(L"DuplicateTokenEx failed: %lu\n", GetLastError());
        CloseHandle(hToken);
        return 1;
    }
    wprintf(L"[+] DuplicateTokenEx succeeded\n");

    // 3. Load the user profile
    PROFILEINFOW profileInfo = { 0 };
    profileInfo.dwSize = sizeof(PROFILEINFOW);
    profileInfo.lpUserName = L"TestUser";   // Must be the same user
    profileInfo.dwFlags = PI_NOUI;          // No UI prompts

    if (!LoadUserProfileW(hPrimaryToken, &profileInfo))
    {
        wprintf(L"LoadUserProfile failed: %lu\n", GetLastError());
        CloseHandle(hPrimaryToken);
        CloseHandle(hToken);
        return 1;
    }
    hProfile = profileInfo.hProfile;        // Save handle for UnloadUserProfile later
    wprintf(L"[+] User profile loaded (handle: %p)\n", hProfile);

    // 4. Create environment block
    if (!CreateEnvironmentBlock(&lpEnvironment, hPrimaryToken, FALSE))
    {
        wprintf(L"CreateEnvironmentBlock failed: %lu\n", GetLastError());
        lpEnvironment = NULL;
    }
    else
    {
        wprintf(L"[+] Environment block created\n");
    }

    // 5. Launch the process
    if (!CreateProcessWithTokenW(
        hPrimaryToken,
        0,
        L"C:\\Windows\\System32\\calc.exe",  
        NULL,
        CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
        lpEnvironment,
        L"C:\\Windows",
        &si,
        &pi))
    {
        wprintf(L"CreateProcessWithTokenW failed: %lu (0x%08lX)\n", GetLastError(), GetLastError());
        goto cleanup;
    }

    wprintf(L"[+] Process created successfully\n[+] PID: %lu\n", pi.dwProcessId);

    WaitForSingleObject(pi.hProcess, INFINITE);
    if (GetExitCodeProcess(pi.hProcess, &exitCode))
        wprintf(L"[+] Exit code: %lu (0x%08lX)\n", exitCode, exitCode);

cleanup:
    // Clean up in reverse order
    if (pi.hThread) CloseHandle(pi.hThread);
    if (pi.hProcess) CloseHandle(pi.hProcess);
    if (lpEnvironment) DestroyEnvironmentBlock(lpEnvironment);
    if (hProfile) UnloadUserProfile(hPrimaryToken, hProfile);  // Unload profile
    if (hPrimaryToken) CloseHandle(hPrimaryToken);
    if (hToken) CloseHandle(hToken);

    return (pi.hProcess ? 0 : 1);
}