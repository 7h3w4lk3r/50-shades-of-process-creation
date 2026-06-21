#include <Windows.h>
#include "ntdll.h"
#include <stdio.h>
#pragma comment(lib, "ntdll.lib")

int main(void)
{
    UNICODE_STRING NtImagePath;

    PRTL_USER_PROCESS_PARAMETERS ProcessParameters = NULL;

    PS_CREATE_INFO CreateInfo;

    PPS_ATTRIBUTE_LIST AttributeList;

    HANDLE hProcess = NULL;
    HANDLE hThread = NULL;

    NTSTATUS Status;

    /* Target image */
    RtlInitUnicodeString(
        &NtImagePath,
        L"\\??\\C:\\Windows\\System32\\calc.exe");

    /* Process parameters */
    Status = RtlCreateProcessParametersEx(
        &ProcessParameters,
        &NtImagePath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        RTL_USER_PROCESS_PARAMETERS_NORMALIZED);

    if (!NT_SUCCESS(Status))
    {
        return 1;
    }

    /* PS_CREATE_INFO */
    ZeroMemory(&CreateInfo, sizeof(CreateInfo));

    CreateInfo.Size = sizeof(CreateInfo);
    CreateInfo.State = PsCreateInitialState;

    /* PS_ATTRIBUTE_LIST */
    AttributeList = (PPS_ATTRIBUTE_LIST)
        RtlAllocateHeap(
            RtlProcessHeap(),
            HEAP_ZERO_MEMORY,
            sizeof(PS_ATTRIBUTE_LIST));

    if (AttributeList == NULL)
    {
        RtlDestroyProcessParameters(ProcessParameters);
        return 1;
    }

    AttributeList->TotalLength =
        sizeof(PS_ATTRIBUTE_LIST) -
        sizeof(PS_ATTRIBUTE);

    AttributeList->Attributes[0].Attribute =
        PS_ATTRIBUTE_IMAGE_NAME;

    AttributeList->Attributes[0].Size =
        NtImagePath.Length;

    AttributeList->Attributes[0].Value =
        (ULONG_PTR)NtImagePath.Buffer;

    /* Create process */
    Status = NtCreateUserProcess(
        &hProcess,
        &hThread,
        PROCESS_ALL_ACCESS,
        THREAD_ALL_ACCESS,
        NULL,
        NULL,
        0,
        0,
        ProcessParameters,
        &CreateInfo,
        AttributeList);

    if (NT_SUCCESS(Status))
    {
        printf("Process created successfully.\n");
        printf("PID: %lu\n", GetProcessId(hProcess));
    }
    else
    {
        printf(
            "NtCreateUserProcess failed: 0x%08X\n",
            (unsigned int)Status);
    }

    /* Cleanup */
    if (hThread)
        CloseHandle(hThread);

    if (hProcess)
        CloseHandle(hProcess);

    RtlFreeHeap(
        RtlProcessHeap(),
        0,
        AttributeList);

    RtlDestroyProcessParameters(
        ProcessParameters);

    return 0;
}