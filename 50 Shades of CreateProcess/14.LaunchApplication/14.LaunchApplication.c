#include <windows.h>
#include <stdio.h>

// Define the function pointer type for LaunchApplicationW
// Signature: HRESULT WINAPI LaunchApplicationW(LPCWSTR appPath, LPCWSTR cmdLine, LPCWSTR workDir, DWORD nShowCmd)
typedef HRESULT(WINAPI* LaunchAppW_t)(LPCWSTR, LPCWSTR, LPCWSTR, DWORD);

int main() {
    // 1. Load pcwutl.dll (Program Compatibility Wizard utility)
    HMODULE hMod = LoadLibraryW(L"pcwutl.dll");
    if (!hMod) {
        printf("Failed to load pcwutl.dll (error: %lu)\n", GetLastError());
        return 1;
    }

    // 2. Get the address of LaunchApplicationW
    LaunchAppW_t pLaunchApp = (LaunchAppW_t)GetProcAddress(hMod, "LaunchApplicationW");
    if (!pLaunchApp) {
        printf("Failed to get LaunchApplicationW address (error: %lu)\n", GetLastError());
        FreeLibrary(hMod);
        return 1;
    }

    printf("LaunchApplicationW address: %p\n", pLaunchApp);

    // 3. Call it to launch calc.exe (may not always work on modern systems)
    // Parameters: app path, command line (NULL), working dir (NULL), nShowCmd (SW_SHOW = 5)
    HRESULT hr = pLaunchApp(L"C:\\Windows\\System32\\calc.exe", NULL, NULL, SW_SHOW);

    if (SUCCEEDED(hr)) {
        printf("LaunchApplicationW succeeded (0x%08lX)\n", hr);
    }
    else {
        printf("LaunchApplicationW failed (0x%08lX)\n", hr);
    }

    // 4. Clean up
    FreeLibrary(hMod);
    return 0;
}