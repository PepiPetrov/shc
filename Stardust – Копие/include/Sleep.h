#ifndef STARDUST_SLEEP_H
#define STARDUST_SLEEP_H

#include <Common.h>

#define SLEEPOBF_BYPASS_NONE 0
#define SLEEPOBF_BYPASS_JMPRAX 0x1
#define SLEEPOBF_BYPASS_JMPRBX 0x2

#define OBF_JMP(i, p)                        \
    if (JmpBypass == SLEEPOBF_BYPASS_JMPRAX) \
    {                                        \
        Rop[i].Rax = U_PTR(p);               \
    }                                        \
    if (JmpBypass == SLEEPOBF_BYPASS_JMPRBX) \
    {                                        \
        Rop[i].Rbx = U_PTR(&p);              \
    }                                        \
    else                                     \
    {                                        \
        Rop[i].Rip = U_PTR(p);               \
    }

#define LDR_GADGET_MODULE_SIZE (0x1000 * 0x1000)
#define LDR_GADGET_HEADER_SIZE (0x1000)

typedef struct _FLOWER_ROPSTART
{
    //
    // NtSignalAndWaitForSingleObject + args
    //
    PVOID Func;
    PVOID Signal;
    PVOID Wait;
    PVOID Alertable;
    PVOID TimeOut;

    //
    // retaddr patching
    //
    PVOID ImgBase;
    PVOID NewBase;
    WORD ssn;

} FLOWER_ROPSTART_PRM, *PFLOWER_ROPSTART_PRM;

EXTERN_C VOID FwPatchRetAddr(PVOID ImgBase, PVOID NewBase);
EXTERN_C VOID FwRopStart(PFLOWER_ROPSTART_PRM Prm);

FUNC VOID EkkoEx(
    IN DWORD TimeOut,
    IN BYTE JmpBypass);

#endif // STARDUST_SLEEP_H
