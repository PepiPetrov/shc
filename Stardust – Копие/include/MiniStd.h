#ifndef STARDUST_H_MINISTD
#define STARDUST_H_MINISTD

#include <Common.h>

#define LCG_A 0x7fffffed
#define LCG_C 0x7fffffc3

INT StringCompareA(LPCSTR String1, LPCSTR String2);
PCHAR *StringTokenA(PCHAR str, PCHAR delimiters);
SIZE_T StringLengthA(LPCSTR String);
SIZE_T StringLengthW(LPCWSTR String);
SIZE_T CharStringToWCharString(PWCHAR Destination, PCHAR Source, SIZE_T MaximumAllowed);

ULONG CustomRtlRandomEx(PULONG Seed);
ULONG RandomNumber32(VOID);
LARGE_INTEGER MillisecondsToLargeInteger(DWORD dwMilliseconds);

INT MemCompare(PVOID s1, PVOID s2, INT len);

PVOID MmGadgetFind(
    _In_ PVOID Memory,
    _In_ SIZE_T Length,
    _In_ PVOID PatternBuffer,
    _In_ SIZE_T PatternLength);

INT StringToInt(LPSTR str);
INT StringNCompareA(LPCSTR String1, LPCSTR String2, SIZE_T n);
VOID StringCopyA(LPSTR dest, LPSTR src);
PVOID CustomReAlloc(PVOID ptr, SIZE_T newSize);

VOID CustomRtlInitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString);


#endif
