#include <SysNative.h>

FUNC NTSTATUS SysNtClose(HANDLE Handle){
    SYSCALL_INVOKE(NtClose, Handle)}

FUNC NTSTATUS SysNtCreateEvent(PHANDLE EventHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, EVENT_TYPE EventType, BOOLEAN InitialState){
    SYSCALL_INVOKE(NtCreateEvent, EventHandle, DesiredAccess, ObjectAttributes, EventType, InitialState)}

FUNC NTSTATUS SysNtWaitForSingleObject(HANDLE Handle, BOOLEAN Alertable, PLARGE_INTEGER Timeout){
    SYSCALL_INVOKE(NtWaitForSingleObject, Handle, Alertable, Timeout)}

FUNC NTSTATUS SysNtAllocateVirtualMemory(HANDLE ProcessHandle, PVOID *BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect){
    SYSCALL_INVOKE(NtAllocateVirtualMemory, ProcessHandle, BaseAddress, ZeroBits, RegionSize, AllocationType, Protect)}

FUNC NTSTATUS SysNtProtectVirtualMemory(HANDLE ProcessHandle, PVOID *BaseAddress, PSIZE_T RegionSize, ULONG NewProtect, PULONG OldProtect)
{
    SYSCALL_INVOKE(NtProtectVirtualMemory, ProcessHandle, BaseAddress, RegionSize, NewProtect, OldProtect)
}
