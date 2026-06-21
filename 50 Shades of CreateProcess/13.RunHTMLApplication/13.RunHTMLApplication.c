#include <windows.h>
#include <stdio.h>

int main()
{
    // 1. Create the .hta file in the temp folder
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    char htaPath[MAX_PATH];
    snprintf(htaPath, sizeof(htaPath), "%s\\launch.hta", tempPath);

    HANDLE hFile = CreateFileA(htaPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Failed to create .hta file (error: %lu)\n", GetLastError());
        return 1;
    }

    // HTA content – uses VBScript to launch calc.exe with full path
    const char* content =
        "<html>\n"
        "<head><title>Launch</title></head>\n"
        "<body>\n"
        "<script language=\"VBScript\">\n"
        "    CreateObject(\"WScript.Shell\").Run \"calc.exe\", 1, 0\n"
        "    window.close()\n"
        "</script>\n"
        "</body>\n"
        "</html>\n";

    DWORD written;
    WriteFile(hFile, content, (DWORD)strlen(content), &written, NULL);
    CloseHandle(hFile);

    printf("HTA file created: %s\n", htaPath);

    // 2. Build the command line for mshta.exe with the .hta path quoted
    char cmdLine[MAX_PATH + 64];
    snprintf(cmdLine, sizeof(cmdLine), "mshta.exe \"%s\"", htaPath);

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    // 3. Launch mshta.exe
    if (!CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("CreateProcess failed (error: %lu)\n", GetLastError());
        DeleteFileA(htaPath);
        return 1;
    }

    printf("mshta.exe launched. Waiting for it to finish...\n");
    WaitForSingleObject(pi.hProcess, INFINITE);  // wait until the HTA closes
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // 4. Clean up
    DeleteFileA(htaPath);
    printf("Done.\n");
    return 0;
}