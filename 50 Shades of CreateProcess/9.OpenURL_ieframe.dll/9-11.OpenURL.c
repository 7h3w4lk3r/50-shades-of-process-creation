#include <windows.h>
#include <stdio.h>
#include <shlobj.h>      // for SHGetFolderPathA, CSIDL_LOCAL_APPDATA

// Correct signature for OpenURL (rundll32 entry point)
typedef void (WINAPI* pOpenURL)(HWND, HINSTANCE, LPCSTR, int);

// Helper: create a temporary .url file in the temp folder
BOOL CreateTempUrlFile(LPCSTR pszUrl, char* pszPathOut, DWORD dwPathSize)
{
    char szTempPath[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, szTempPath)) {
        printf("GetTempPathA failed\n");
        return FALSE;
    }

    // Generate a unique filename
    char szFileName[MAX_PATH];
    if (!GetTempFileNameA(szTempPath, "URL", 0, szFileName)) {
        printf("GetTempFileNameA failed\n");
        return FALSE;
    }

    // Write the .url content
    HANDLE hFile = CreateFileA(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("CreateFileA failed (error: %lu)\n", GetLastError());
        return FALSE;
    }

    char szContent[512];
    // .url format: [InternetShortcut] section with URL=...
    // Note: we use file:/// with forward slashes
    snprintf(szContent, sizeof(szContent), "[InternetShortcut]\r\nURL=%s\r\n", pszUrl);

    DWORD dwWritten;
    if (!WriteFile(hFile, szContent, (DWORD)strlen(szContent), &dwWritten, NULL) || dwWritten != strlen(szContent)) {
        printf("WriteFile failed\n");
        CloseHandle(hFile);
        return FALSE;
    }

    CloseHandle(hFile);

    // Copy the path to output
    strncpy_s(pszPathOut, dwPathSize, szFileName, _TRUNCATE);
    return TRUE;
}

int main()
{
    // 1. Load the DLL (choose either ieframe or shdocvw)
    HMODULE hMod = LoadLibraryW(L"ieframe.dll");    // shade #9
    //HMODULE hMod = LoadLibraryW(L"shdocvw.dll");  // shade #10
    //HMODULE hMod = LoadLibraryW(L"url.dll");      // shade #11
    if (!hMod) {
        printf("Failed to load ieframe.dll (error: %lu)\n", GetLastError());
        return 1;
    }

    // 2. Get the OpenURL export
    pOpenURL func = (pOpenURL)GetProcAddress(hMod, "OpenURL");
    if (!func) {
        printf("Failed to get OpenURL address (error: %lu)\n", GetLastError());
        FreeLibrary(hMod);
        return 1;
    }

    // 3. Create a temporary .url file that points to calc.exe
    char szUrlFile[MAX_PATH];
    if (!CreateTempUrlFile("file:///C:/Windows/System32/calc.exe", szUrlFile, sizeof(szUrlFile))) {
        FreeLibrary(hMod);
        return 1;
    }

    printf("Temporary .url file created: %s\n", szUrlFile);

    // 4. Call OpenURL with the .url file path
    //    Parameters: HWND, HINSTANCE (unused), path to .url file, nShowCmd (1 = SW_SHOWNORMAL)
    func(NULL, NULL, szUrlFile, 1);

    printf("OpenURL called with .url file\n");

    // 5. Wait a moment for the file to be read, then delete it
    Sleep(2000);   // give the system time to launch
    if (!DeleteFileA(szUrlFile)) {
        printf("Failed to delete temporary file (error: %lu)\n", GetLastError());
    }

    FreeLibrary(hMod);
    return 0;
}