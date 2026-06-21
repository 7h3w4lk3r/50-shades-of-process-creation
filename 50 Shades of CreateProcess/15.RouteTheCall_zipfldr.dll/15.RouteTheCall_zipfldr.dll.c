#include <windows.h>
#include <stdio.h>

// Try all plausible signatures
typedef void (WINAPI* pRouteTheCall_A)(HWND, HINSTANCE, LPCSTR, int);   // ANSI version
typedef void (WINAPI* pRouteTheCall_W)(HWND, HINSTANCE, LPCWSTR, int);  // Wide version
typedef void (WINAPI* pRouteTheCall_3)(HWND, LPCSTR, int);              // 3-param variant
typedef void (WINAPI* pRouteTheCall_4)(HINSTANCE, HINSTANCE, LPCSTR, int); // swapped first two

void SafeCall(void* func, int variant, const char* desc) {
    __try {
        // We'll use the ANSI signature as the primary guess
        if (variant == 1) {
            pRouteTheCall_A f = (pRouteTheCall_A)func;
            f(NULL, NULL, "C:\\Windows\\System32\\calc.exe", SW_SHOW);
            printf("[%s] Call succeeded (no crash)\n", desc);
        }
        else if (variant == 2) {
            pRouteTheCall_W f = (pRouteTheCall_W)func;
            f(NULL, NULL, L"C:\\Windows\\System32\\calc.exe", SW_SHOW);
            printf("[%s] Call succeeded (no crash)\n", desc);
        }
        else if (variant == 3) {
            pRouteTheCall_3 f = (pRouteTheCall_3)func;
            f(NULL, "C:\\Windows\\System32\\calc.exe", SW_SHOW);
            printf("[%s] Call succeeded (no crash)\n", desc);
        }
        else if (variant == 4) {
            pRouteTheCall_4 f = (pRouteTheCall_4)func;
            f(NULL, NULL, "C:\\Windows\\System32\\calc.exe", SW_SHOW);
            printf("[%s] Call succeeded (no crash)\n", desc);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        printf("[%s] EXCEPTION CAUGHT (access violation or other)\n", desc);
    }
}

int main() {
    HMODULE hMod = LoadLibraryW(L"zipfldr.dll");
    if (!hMod) {
        printf("LoadLibrary failed: %lu\n", GetLastError());
        return 1;
    }

    void* func = GetProcAddress(hMod, "RouteTheCall");
    if (!func) {
        // Try decorated name
        func = GetProcAddress(hMod, "RouteTheCall@16");
    }
    if (!func) {
        printf("GetProcAddress failed\n");
        FreeLibrary(hMod);
        return 1;
    }
    printf("Function address: %p\n", func);

    // Try each signature
    SafeCall(func, 1, "ANSI (HWND,HINSTANCE,LPCSTR,int)");
    SafeCall(func, 2, "Wide (HWND,HINSTANCE,LPCWSTR,int)");
    SafeCall(func, 3, "3-param (HWND,LPCSTR,int)");
    SafeCall(func, 4, "Swapped (HINSTANCE,HINSTANCE,LPCSTR,int)");

    FreeLibrary(hMod);
    return 0;
}