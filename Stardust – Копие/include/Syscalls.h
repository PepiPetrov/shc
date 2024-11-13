#ifndef HELLHALL_H
#define HELLHALL_H

#include <Common.h>

#define RANGE 0x1E // search in 30 bytes to search for `syscall` instruction

// FROM Syscalls.c
FUNC BOOL InitilizeSysFunc(IN ULONG uSysFuncHash);
FUNC VOID GetSysFuncStruct(OUT PSYS_FUNC psF);

// FROM Syscalls.asm
EXTERN_C VOID SetConfig(IN PSYS_CONFIG Config);
EXTERN_C NTSTATUS Invoke(...);

FUNC VOID PrepareSyscall(SYS_FUNC sF);

#define SYSCALL_SETUP(FunctionName)                 \
    InitilizeSysFunc(HASH_STR(#FunctionName)); \
    GetSysFuncStruct(&Instance()->Syscall.FunctionName);

#endif // !HELLHALL_H
