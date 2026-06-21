#include <windows.h>
#include <stdio.h>

// Correct signature for FileProtocolHandlerA
// HRESULT WINAPI FileProtocolHandlerA(HWND hWnd, HINSTANCE hInst, LPCSTR pszUrl, int nShowCmd);
typedef HRESULT(WINAPI* pFileProtocolHandler)(HWND, HINSTANCE, LPCSTR, int);

int main()
{
    // Load url.dll
    HMODULE hMod = LoadLibraryW(L"url.dll");
    if (!hMod) {
        printf("Failed to load url.dll (error: %lu)\n", GetLastError());
        return 1;
    }

    // Get the address of FileProtocolHandlerA with the correct signature
    pFileProtocolHandler func = (pFileProtocolHandler)GetProcAddress(hMod, "FileProtocolHandlerA");
    if (!func) {
        printf("Failed to get FileProtocolHandlerA address (error: %lu)\n", GetLastError());
        FreeLibrary(hMod);
        return 1;
    }

    // Try with a proper file:// URL (what the handler expects)
    // nShowCmd = 1 = SW_SHOWNORMAL
    HRESULT result = func(NULL, NULL, "file:///C:/Windows/System32/calc.exe", 1);
    printf("FileProtocolHandlerA returned: 0x%08lX\n", result);

    // Also try with a bare path (may or may not work)
    HRESULT result2 = func(NULL, NULL, "C:\\Windows\\System32\\calc.exe", 1);
    printf("FileProtocolHandlerA (bare path) returned: 0x%08lX\n", result2);

    FreeLibrary(hMod);
    return 0;
}