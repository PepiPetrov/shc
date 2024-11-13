#ifndef STARDUST_DEFS_H
#define STARDUST_DEFS_H

#include <Common.h>

typedef struct _BUFFER
{
    PVOID Buffer;
    ULONG Length;
} BUFFER, *PBUFFER;

typedef struct
{
    DWORD Length;
    DWORD MaximumLength;
    PVOID Buffer;
} USTRING;

typedef struct _SysFunc
{

    PVOID pInst;    // address of a 'syscall' instruction in ntdll.dll
    PBYTE pAddress; // address of the syscall
    WORD wSSN;      // syscall number
} SYS_FUNC, *PSYS_FUNC;

typedef struct _SYS_CONFIG
{
    PVOID Adr; /* indirect syscall instruction address */
    WORD Ssn;  /* syscall service number */
} SYS_CONFIG, *PSYS_CONFIG;

typedef struct _PACKAGE
{
    PCHAR TaskID;
    PVOID Buffer;
    SIZE_T Length;
    BOOL Destroy; /* destroy this package after sending */
} PACKAGE, *PPACKAGE;

//
// Hashing defines
//
#define H_MAGIC_KEY 5381
#define H_MAGIC_SEED 5
#define H_MODULE_NTDLL 0x70e61753
#define H_MODULE_KERNEL32 0xadd31df0

#endif // STARDUST_DEFS_H
