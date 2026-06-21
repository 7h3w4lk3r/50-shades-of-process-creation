#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>

int main(void) {
    // ShellExecute returns an HINSTANCE; values > 32 indicate success.
    HINSTANCE result = ShellExecuteW(
        NULL,                       // owner window
        L"open",                    // verb
        L"C:\\Windows\\System32\\calc.exe",  // file
        NULL,                       // parameters
        NULL,                       // directory
        SW_SHOW                     // show state
    );

    if ((INT_PTR)result > 32) {
        printf("Calculator launched successfully.\n");
    }
    else {
        DWORD err = GetLastError();
        printf("ShellExecute failed with error %lu\n", err);
    }
    return 0;
}