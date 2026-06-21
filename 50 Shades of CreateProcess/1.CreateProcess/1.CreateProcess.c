#include <windows.h>
#include <stdio.h>

int main() {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    
    wchar_t cmdLine[] = L"C:\\Windows\\System32\\calc.exe";
    
    if (CreateProcessW(NULL, cmdLine, NULL, NULL, 
                       FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("Process created! PID: %d\n", pi.dwProcessId);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    return 0;
}