#include <CfgManip.h>

FUNC void CfgPrivateAddressAdd(
    _In_ HANDLE Process,
    _In_ PVOID Address,
    _In_ ULONG Size)
{
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

    SysNtSetInformationVirtualMemory(Process, VmCfgCallTargetInformation, 1, &MemRange, &VmInfo, sizeof(VmInfo));
}
