#include <windows.h>
#include <stdio.h>

int main(void) {
    // Force Wide/Unicode structural equivalents to Win32_ProcessStartup
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Hide the window frame
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    // Use a Wide-string literal (prefixed with L) so it compiles as UTF-16 wchar_t
    wchar_t commandLine[] = L"C:\\Windows\\System32\\cmd.exe /c calc.exe";

    wprintf(L"[*] Launching hidden process via explicit Wide-character API...\n");

    // Explicitly call the Wide/Unicode engine variant directly
    BOOL success = CreateProcessW(
        NULL,           // Let Windows resolve via commandLine parameter
        commandLine,    // Command string array (wchar_t)
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (success) {
        printf("[+] Process started successfully.\n");
        printf("[+] Process ID (PID): %lu\n", pi.dwProcessId);

        // Clean up open handles safely
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return 0;
    }
    else {
        printf("[-] Failed to start process. Error code: %lu\n", GetLastError());
        return 1;
    }
}