#include <windows.h>
#include <stdio.h>
#include <shlobj.h>

// Force strict GUID macro generation for raw C structures
#include <initguid.h>

// Define the root Shell Automation engine CLSID cleanly
DEFINE_GUID(CLSID_Shell, 0x13709620, 0xC279, 0x11CE, 0xA4, 0x9E, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

int main(void) {
    HRESULT hr;
    IDispatch* pShellApp = NULL; // FIX: Maintained strictly as a universal IDispatch pointer
    DISPID dispidShellExec;
    VARIANT vtResult;

    // Allocate parameters as an array block to satisfy VARIANTARG* verification
    VARIANT rgvarg[5];
    DISPPARAMS params = { 0 };

    BSTR bstrMethod = NULL;
    BSTR bstrFile = NULL, bstrArgs = NULL, bstrDir = NULL, bstrVerb = NULL;

    // Initialize COM Apartment Threading Model
    CoInitialize(NULL);

    printf("[*] Connecting to root In-Process Shell Application host...\n");

    // FIX: Requesting IID_IDispatch directly bypasses the 0x80004002 (E_NOINTERFACE) restriction
    hr = CoCreateInstance(&CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, &IID_IDispatch, (void**)&pShellApp);
    if (FAILED(hr) || !pShellApp) {
        printf("[-] CoCreateInstance Shell.Application failed: 0x%08lX\n", hr);
        goto cleanup;
    }

    // 2. Resolve the explicit DISPID function code for ShellExecute
    bstrMethod = SysAllocString(L"ShellExecute");
    hr = pShellApp->lpVtbl->GetIDsOfNames(pShellApp, &IID_NULL, &bstrMethod, 1, LOCALE_USER_DEFAULT, &dispidShellExec);
    if (FAILED(hr)) {
        printf("[-] GetIDsOfNames ShellExecute failed on target interface: 0x%08lX\n", hr);
        goto cleanup;
    }

    // 3. Define target payload tokens
    bstrFile = SysAllocString(L"C:\\Windows\\System32\\cmd.exe");
    bstrArgs = SysAllocString(L"/c calc.exe");
    bstrDir = SysAllocString(L"");
    bstrVerb = SysAllocString(L"open");

    if (!bstrFile || !bstrArgs || !bstrDir || !bstrVerb) {
        printf("[-] SysAllocString failed for payload tracking metrics.\n");
        goto cleanup;
    }

    // 4. Structure properties into the array layout configuration in precise reverse sequence
    // Index 0 -> Show Window configuration flag layout (SW_SHOWNORMAL = 1)
    VariantInit(&rgvarg[0]);
    rgvarg[0].vt = VT_I4;
    rgvarg[0].lVal = SW_SHOWNORMAL; // Change to SW_HIDE to run background tasks silently

    // Index 1 -> Action Verb string
    VariantInit(&rgvarg[1]);
    rgvarg[1].vt = VT_BSTR;
    rgvarg[1].bstrVal = bstrVerb;

    // Index 2 -> Target working directory path
    VariantInit(&rgvarg[2]);
    rgvarg[2].vt = VT_BSTR;
    rgvarg[2].bstrVal = bstrDir;

    // Index 3 -> Core payload command argument parameter buffer
    VariantInit(&rgvarg[3]);
    rgvarg[3].vt = VT_BSTR;
    rgvarg[3].bstrVal = bstrArgs;

    // Index 4 -> Target file engine binary layout vector
    VariantInit(&rgvarg[4]);
    rgvarg[4].vt = VT_BSTR;
    rgvarg[4].bstrVal = bstrFile;

    // Bind array descriptor pointers cleanly to clear compiler warnings
    params.rgvarg = rgvarg;
    params.cArgs = 5;
    params.rgdispidNamedArgs = NULL;
    params.cNamedArgs = 0;

    VariantInit(&vtResult);

    // 5. Invoke decoupled process execution via the system DCOM coordinator subsystem context
    printf("[*] Launching process out-of-tree via automated Shell dispatch channel...\n");
    hr = pShellApp->lpVtbl->Invoke(pShellApp, dispidShellExec, &IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &vtResult, NULL, NULL);

    if (SUCCEEDED(hr)) {
        printf("[+] Success! Process decoupled. calc.exe is running out-of-process via decoupled context.\n");
    }
    else {
        printf("[-] Decoupled process execution invocation failed: 0x%08lX\n", hr);
    }

    VariantClear(&vtResult);

cleanup:
    // 6. Non-overlapping interface release mapping to prevent leaks
    if (pShellApp) pShellApp->lpVtbl->Release(pShellApp);

    if (bstrMethod) SysFreeString(bstrMethod);
    if (bstrFile) SysFreeString(bstrFile);
    if (bstrArgs) SysFreeString(bstrArgs);
    if (bstrDir) SysFreeString(bstrDir);
    if (bstrVerb) SysFreeString(bstrVerb);

    CoUninitialize();
    return 0;
}
