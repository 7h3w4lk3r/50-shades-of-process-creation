#include <windows.h>
#include <stdio.h>

// --- CLSID_InteractiveUser: {8CEC58E7-07A1-11D9-B15E-000D56BFE6EE}
static const GUID CLSID_InteractiveUser = {
    0x8cec58e7, 0x07a1, 0x11d9,
    {0xb1, 0x5e, 0x00, 0x0d, 0x56, 0xbf, 0xe6, 0xee}
};

// --- IID_IHxInteractiveUser: {8CEC595B-07A1-11D9-B15E-000D56BFE6EE}
static const GUID IID_IHxInteractiveUser = {
    0x8cec595b, 0x07a1, 0x11d9,
    {0xb1, 0x5e, 0x00, 0x0d, 0x56, 0xbf, 0xe6, 0xee}
};

// --- IHxInteractiveUser interface (vtable layout) ---
typedef struct IHxInteractiveUser IHxInteractiveUser;

typedef struct {
    // IUnknown
    HRESULT(STDMETHODCALLTYPE* QueryInterface)(IHxInteractiveUser* This, const GUID* riid, void** ppvObject);
    ULONG(STDMETHODCALLTYPE* AddRef)(IHxInteractiveUser* This);
    ULONG(STDMETHODCALLTYPE* Release)(IHxInteractiveUser* This);

    // IHxInteractiveUser method: Execute
    HRESULT(STDMETHODCALLTYPE* Execute)(IHxInteractiveUser* This, LPWSTR pcUrl);
} IHxInteractiveUserVtbl;

struct IHxInteractiveUser {
    IHxInteractiveUserVtbl* lpVtbl;
};

int main() {
    HRESULT hr;
    IHxInteractiveUser* pHelp = NULL;

    // Initialize COM
    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printf("CoInitialize failed: 0x%08lX\n", hr);
        return 1;
    }

    // Create the InteractiveUser COM object
    hr = CoCreateInstance(
        &CLSID_InteractiveUser,
        NULL,
        CLSCTX_ALL,
        &IID_IHxInteractiveUser,
        (void**)&pHelp
    );

    if (FAILED(hr)) {
        printf("CoCreateInstance failed: 0x%08lX\n", hr);
        CoUninitialize();
        return 1;
    }

    printf("IHxInteractiveUser created successfully.\n");

    // Execute calc.exe – method expects a file:// URL
    LPWSTR pcUrl = L"file:///C:/Windows/System32/calc.exe";
    hr = pHelp->lpVtbl->Execute(pHelp, pcUrl);

    if (SUCCEEDED(hr)) {
        printf("Execute succeeded. calc.exe should be running.\n");
    }
    else {
        printf("Execute failed: 0x%08lX\n", hr);
    }

    // Cleanup
    pHelp->lpVtbl->Release(pHelp);
    CoUninitialize();

    return 0;
}