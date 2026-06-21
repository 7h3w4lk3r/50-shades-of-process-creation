#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>

// ------------------------------------------------------------------
// Structure that must be placed in the file mapping
// (Offsets are critical – do not reorder fields)
// ------------------------------------------------------------------
typedef struct _RUNASNEWUSERPARAM {
    WCHAR eventName[0x38];          // Name of an event to signal when done
    WCHAR execFile[MAX_PATH];       // Path to the executable
    WCHAR arguments[MAX_PATH];      // Command-line arguments (optional)
    UCHAR padding[0x600];           // Unused padding to align offsets
    WCHAR directory[MAX_PATH - 4];  // Working directory (optional)
} RUNASNEWUSERPARAM;

// Function pointer type for RunAsNewUser_RunDLLW
typedef int (WINAPI* pRunAsNewUser_RunDLLW)(HWND, HINSTANCE, LPCWSTR, int);

// ------------------------------------------------------------------
int main(void)
{
    HMODULE hShell = NULL;
    pRunAsNewUser_RunDLLW func = NULL;
    HANDLE hFileMap = NULL;
    HANDLE hEvent = NULL;
    RUNASNEWUSERPARAM* pParam = NULL;
    int result = -1;

    // 1. Load shell32.dll
    hShell = LoadLibraryW(L"shell32.dll");
    if (!hShell) {
        wprintf(L"LoadLibraryW failed: %lu\n", GetLastError());
        goto cleanup;
    }

    // 2. Get the address of RunAsNewUser_RunDLLW
    func = (pRunAsNewUser_RunDLLW)GetProcAddress(hShell, "RunAsNewUser_RunDLLW");
    if (!func) {
        wprintf(L"GetProcAddress failed: %lu\n", GetLastError());
        goto cleanup;
    }
    wprintf(L"[+] RunAsNewUser_RunDLLW found at %p\n", func);

    // 3. Create a named file mapping (shared memory)
    LPCWSTR fileMapName = L"MyRunAsNewUserMap";
    hFileMap = CreateFileMappingW(
        INVALID_HANDLE_VALUE,   // Use paging file
        NULL,                   // Default security
        PAGE_READWRITE,         // Read/write access
        0,                      // High-order size
        sizeof(RUNASNEWUSERPARAM), // Low-order size
        fileMapName             // Name of the mapping
    );
    if (!hFileMap) {
        wprintf(L"CreateFileMappingW failed: %lu\n", GetLastError());
        goto cleanup;
    }

    // 4. Map a view of the file mapping into our process
    pParam = (RUNASNEWUSERPARAM*)MapViewOfFile(
        hFileMap,
        FILE_MAP_WRITE,         // Write access
        0, 0, 0                 // Map entire file
    );
    if (!pParam) {
        wprintf(L"MapViewOfFile failed: %lu\n", GetLastError());
        goto cleanup;
    }

    // 5. Create an event that the function will signal when done
    LPCWSTR eventName = L"MyRunAsNewUserEvent";
    hEvent = CreateEventW(NULL, FALSE, FALSE, eventName);
    if (!hEvent) {
        wprintf(L"CreateEventW failed: %lu\n", GetLastError());
        goto cleanup;
    }

    // 6. Fill the RUNASNEWUSERPARAM structure in the shared memory
    wcscpy_s(pParam->eventName, 0x38, eventName);              // Event name
    wcscpy_s(pParam->execFile, MAX_PATH, L"C:\\Windows\\System32\\calc.exe");
    pParam->arguments[0] = L'\0';                              // No arguments
    wcscpy_s(pParam->directory, MAX_PATH - 4, L"C:\\Windows");

    wprintf(L"[+] Shared memory prepared:\n");
    wprintf(L"    Event:   %ls\n", pParam->eventName);
    wprintf(L"    Exec:    %ls\n", pParam->execFile);
    wprintf(L"    Dir:     %ls\n", pParam->directory);

    // 7. Call RunAsNewUser_RunDLLW – pass the *name* of the file mapping
    result = func(NULL, NULL, fileMapName, SW_SHOW);
    wprintf(L"[+] RunAsNewUser_RunDLLW returned: %d (0x%08X)\n", result, result);

    // 8. (Optional) Wait for the event to confirm the function has finished
    //    WaitForSingleObject(hEvent, INFINITE);
    //    wprintf(L"[+] Event signalled – function completed.\n");

cleanup:
    // 9. Clean up resources
    if (pParam)
        UnmapViewOfFile(pParam);
    if (hEvent)
        CloseHandle(hEvent);
    if (hFileMap)
        CloseHandle(hFileMap);
    if (hShell)
        FreeLibrary(hShell);

    return (result == 0) ? 0 : 1;
}