#include <windows.h>
#include <shellapi.h>
#include <stdio.h>

int main(void) {
    SHELLEXECUTEINFOW sei = { 0 };
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS; // Retrieve process handle
    sei.lpVerb = L"open";
    sei.lpFile = L"C:\\Windows\\System32\\calc.exe";
    sei.nShow = SW_SHOW;

    if (ShellExecuteExW(&sei)) {
        printf("Process handle: %p\n", (void*)sei.hProcess);
        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);
    }
    else {
        // Retrieve error code for diagnosis
        DWORD err = GetLastError();
        printf("ShellExecuteEx failed with error %lu\n", err);
    }
    return 0;
}