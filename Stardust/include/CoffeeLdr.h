#ifndef STARDUST_COFFELDR_COFFELDR_H
#define STARDUST_COFFELDR_COFFELDR_H

#include <Common.h>

#if defined(__x86_64__) || defined(_WIN64)

#define COFF_PREP_SYMBOL 0xec598a48
#define COFF_PREP_SYMBOL_SIZE 6

#define COFF_PREP_BEACON 0x353400b0
#define COFF_PREP_BEACON_SIZE (COFF_PREP_SYMBOL_SIZE + 6)

#endif

#define BEACON_API_COUNT 25

typedef struct _COFF_FILE_HEADER
{
    UINT16 Machine;
    UINT16 NumberOfSections;
    UINT32 TimeDateStamp;
    UINT32 PointerToSymbolTable;
    UINT32 NumberOfSymbols;
    UINT16 SizeOfOptionalHeader;
    UINT16 Characteristics;
} COFF_FILE_HEADER, *PCOFF_FILE_HEADER;

/* AMD64  should always be here */
#define MACHINETYPE_AMD64 0x8664

#pragma pack(push, 1)

typedef struct _COFF_SECTION
{
    CHAR Name[8];
    UINT32 VirtualSize;
    UINT32 VirtualAddress;
    UINT32 SizeOfRawData;
    UINT32 PointerToRawData;
    UINT32 PointerToRelocations;
    UINT32 PointerToLineNumbers;
    UINT16 NumberOfRelocations;
    UINT16 NumberOfLinenumbers;
    UINT32 Characteristics;
} COFF_SECTION, *PCOFF_SECTION;

typedef struct _COFF_RELOC
{
    UINT32 VirtualAddress;
    UINT32 SymbolTableIndex;
    UINT16 Type;
} COFF_RELOC, *PCOFF_RELOC;

typedef struct _COFF_SYMBOL
{
    union
    {
        CHAR Name[8];
        UINT32 Value[2];
    } First;

    UINT32 Value;
    UINT16 SectionNumber;
    UINT16 Type;
    UINT8 StorageClass;
    UINT8 NumberOfAuxSymbols;
} COFF_SYMBOL, *PCOFF_SYMBOL;

typedef struct _SECTION_MAP
{
    PCHAR Ptr;
    SIZE_T Size;
} SECTION_MAP, *PSECTION_MAP;

typedef struct _COFFEE
{
    PVOID Data;
    PCOFF_FILE_HEADER Header;
    PCOFF_SECTION Section;
    PCOFF_RELOC Reloc;
    PCOFF_SYMBOL Symbol;

    PSECTION_MAP SecMap;
    PCHAR FunMap;
} COFFEE, *PCOFFEE;

typedef struct
{
    UINT_PTR NameHash;
    PVOID Pointer;
} COFFAPIFUNC;

void CoffeeLdr(PCHAR EntryName, PVOID CoffeeData, PVOID ArgData, SIZE_T ArgSize);
BOOL CoffeeExecuteFunction(PCOFFEE Coffee, PCHAR Function, PVOID Argument, SIZE_T Size);
BOOL CoffeeProcessSections(PCOFFEE Coffee);
BOOL CoffeeCleanup(PCOFFEE Coffee);

#endif
