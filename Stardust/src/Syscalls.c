#include <Syscalls.h>

FUNC BOOL InitilizeSysFunc(IN ULONG uSysFuncHash)
{
    STARDUST_INSTANCE

    if (!uSysFuncHash)
        return FALSE;

    Instance()->Syscall.Global.pAddress = LdrFunction(Instance()->Modules.Ntdll, uSysFuncHash);

    DWORD j = 0;

    while (TRUE)
    {

        //  WE REACHED `ret` INSTRUCTION - THAT IS TOO FAR DOWN
        if (*((PBYTE)Instance()->Syscall.Global.pAddress + j) == 0xC3 && !Instance()->Syscall.Global.pInst)
            return FALSE;

        //  SEARCHING FOR
        //      MOV R10, RCX
        //      MOV RCX, <SSN>
        if (*((PBYTE)Instance()->Syscall.Global.pAddress + j + 0x00) == 0x4C &&
            *((PBYTE)Instance()->Syscall.Global.pAddress + j + 0x01) == 0x8B &&
            *((PBYTE)Instance()->Syscall.Global.pAddress + j + 0x02) == 0xD1 &&
            *((PBYTE)Instance()->Syscall.Global.pAddress + j + 0x03) == 0xB8)
        {

            BYTE low = *((PBYTE)Instance()->Syscall.Global.pAddress + j + 0x04);
            BYTE high = *((PBYTE)Instance()->Syscall.Global.pAddress + j + 0x05);

            // GETTING THE SSN
            Instance()->Syscall.Global.wSSN = (high << 0x08) | low;

            // GETTING THE ADDRESS OF THE `syscall` INSTRUCTION, SO THAT WE CAN JUMP TO LATER
            for (DWORD z = 0, x = 1; z <= RANGE; z++, x++)
            {
                if (*((PBYTE)Instance()->Syscall.Global.pAddress + j + z) == 0x0F && *((PBYTE)Instance()->Syscall.Global.pAddress + j + x) == 0x05 && *((PBYTE)Instance()->Syscall.Global.pAddress + j + x + 1) == 0xc3)
                {
                    Instance()->Syscall.Global.pInst = (Instance()->Syscall.Global.pAddress + j + z);
                    break;
                }
            }

            if (Instance()->Syscall.Global.wSSN && Instance()->Syscall.Global.pInst)
                return TRUE;
            else
                return FALSE;
        }

        // HOOKED
        j++;
    }

    return FALSE;
}

FUNC VOID GetSysFuncStruct(OUT PSYS_FUNC psF)
{
    STARDUST_INSTANCE

    psF->pAddress = Instance()->Syscall.Global.pAddress;
    psF->pInst = Instance()->Syscall.Global.pInst;
    psF->wSSN = Instance()->Syscall.Global.wSSN;
}

FUNC VOID PrepareSyscall(SYS_FUNC sF)
{
    STARDUST_INSTANCE

    Instance()->Syscall.SysCfg.Ssn = sF.wSSN;
    Instance()->Syscall.SysCfg.Adr = sF.pInst;
    SetConfig(&Instance()->Syscall.SysCfg);
}
