#ifndef STARDUST_SYSNATIVE_H
#define STARDUST_SYSNATIVE_H

#include <Common.h>

#define SYSCALL_INVOKE(SYS_NAME, ...)             \
    STARDUST_INSTANCE                             \
    PrepareSyscall(Instance()->Syscall.SYS_NAME); \
    return Invoke(__VA_ARGS__);

FUNC NTSTATUS SysNtClose(HANDLE Handle);
FUNC NTSTATUS SysNtCreateEvent(PHANDLE EventHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, EVENT_TYPE EventType, BOOLEAN InitialState);
FUNC NTSTATUS SysNtSignalAndWaitForSingleObject(HANDLE SignalHandle, HANDLE WaitHandle, BOOLEAN Alertable, PLARGE_INTEGER Timeout);
FUNC NTSTATUS SysNtWaitForSingleObject(HANDLE Handle, BOOLEAN Alertable, PLARGE_INTEGER Timeout);
FUNC NTSTATUS SysNtAllocateVirtualMemory(HANDLE ProcessHandle, PVOID *BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);
FUNC NTSTATUS SysNtProtectVirtualMemory(HANDLE ProcessHandle, PVOID *BaseAddress, PSIZE_T RegionSize, ULONG NewProtect, PULONG OldProtect);

#endif
