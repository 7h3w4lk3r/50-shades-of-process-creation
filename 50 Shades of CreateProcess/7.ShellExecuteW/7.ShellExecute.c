#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>

// Define the function pointer type
typedef void (WINAPI* pShellExec_RunDLLW)(HWND hwnd, HINSTANCE hinst, LPWSTR lpCmdLine, int nShowCmd);

int main(void)
{
    HMODULE hShell = NULL;
    pShellExec_RunDLLW func = NULL;

    // 1. Load shell32.dll
    hShell = LoadLibraryW(L"shell32.dll");
    if (!hShell)
    {
        wprintf(L"LoadLibraryW failed: %lu\n", GetLastError());
        return 1;
    }

    // 2. Get the address of ShellExec_RunDLLW
    func = (pShellExec_RunDLLW)GetProcAddress(hShell, "ShellExec_RunDLLW");
    if (!func)
    {
        wprintf(L"GetProcAddress failed: %lu\n", GetLastError());
        FreeLibrary(hShell);
        return 1;
    }

    wprintf(L"[+] ShellExec_RunDLLW found at %p\n", func);

    // 3. Prepare the command line (must be writable, so use an array)
    WCHAR cmdLine[] = L"C:\\Windows\\System32\\calc.exe";

    // 4. Call the function – it launches the process
    //    Parameters: hwnd (NULL), hinst (NULL), command line, nShowCmd (SW_SHOW)
    func(NULL, NULL, cmdLine, SW_SHOW);

    wprintf(L"[+] Process launched. Check your calculator.\n");

    // 5. Clean up
    FreeLibrary(hShell);

    return 0;
}