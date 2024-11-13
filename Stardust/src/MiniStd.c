#include <MiniStd.h>

FUNC INT StringCompareA(_In_ LPCSTR String1, _In_ LPCSTR String2)
{
    for (; *String1 == *String2; String1++, String2++)
    {
        if (*String1 == '\0')
            return 0;
    }

    return ((*(LPCSTR)String1 < *(LPCSTR)String2) ? -1 : +1);
}

FUNC VOID StringCopyA(LPSTR dest, LPSTR src)
{
    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = '\0'; // Null-terminate the destination string
}

FUNC SIZE_T StringLengthA(_In_ LPCSTR String)
{
    LPCSTR String2;

    for (String2 = String; *String2; ++String2)
        ;

    return (String2 - String);
}

FUNC SIZE_T StringLengthW(_In_ LPCWSTR String)
{
    LPCWSTR String2;

    for (String2 = String; *String2; ++String2)
        ;

    return (String2 - String);
}

FUNC SIZE_T CharStringToWCharString(PWCHAR Destination, PCHAR Source, SIZE_T MaximumAllowed)
{
    INT Length = (INT)MaximumAllowed;

    while (--Length >= 0)
    {
        if (!(*Destination++ = *Source++))
            return MaximumAllowed - Length - 1;
    }

    return MaximumAllowed - Length;
}

FUNC PVOID CustomReAlloc(PVOID ptr, SIZE_T newSize)
{
    STARDUST_INSTANCE

    if (newSize == 0)
    {
        // If newSize is 0, free the memory block and return NULL.
        if (ptr != NULL)
        {
            Instance()->Win32.RtlFreeHeap(Instance()->Heap, 0, ptr);
        }
        return NULL;
    }

    if (ptr == NULL)
    {
        // If ptr is NULL, allocate new memory of the specified size.
        return Instance()->Win32.RtlAllocateHeap(Instance()->Heap, HEAP_ZERO_MEMORY, newSize);
    }
    else
    {
        // Resize the memory block, preserving its contents.
        return Instance()->Win32.RtlReAllocateHeap(Instance()->Heap, HEAP_ZERO_MEMORY, ptr, newSize);
    }
}

FUNC VOID StrcpyA(PCHAR dest, PCHAR src)
{
    while (*src)
    {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

// Custom implementation of strchr (returns pointer to first occurrence of character in string)
FUNC PCHAR StrchrA(PCHAR str, INT c)
{
    while (*str)
    {
        if (*str == c)
        {
            return (PCHAR)str;
        }
        str++;
    }
    return NULL;
}

FUNC PCHAR *StringTokenA(PCHAR str, PCHAR delimiters)
{
    STARDUST_INSTANCE

    PCHAR token;
    PCHAR stringCopy;
    SIZE_T len = StringLengthA(str);
    PCHAR *tokens = NULL;
    INT numTokens = 0;

    // Create a copy of the string to avoid modifying the original
    stringCopy = (PCHAR)Instance()->Win32.RtlAllocateHeap(Instance()->Heap, HEAP_ZERO_MEMORY, (len + 1) * sizeof(char));
    if (stringCopy == NULL)
    {
        return NULL;
    }
    StrcpyA(stringCopy, str);

    // Tokenize the string copy
    token = stringCopy;
    while (*token)
    {
        // Skip leading delimiters
        while (*token && StrchrA(delimiters, *token))
        {
            token++;
        }

        // Find the end of the token
        char *end = token;
        while (*end && !StrchrA(delimiters, *end))
        {
            end++;
        }

        // Null-terminate the token
        if (*end)
        {
            *end = '\0';
            end++;
        }

        // Add the token if it's not empty
        if (*token)
        {
            numTokens++;
            char **newTokens = (PCHAR *)Instance()->Win32.RtlAllocateHeap(Instance()->Heap, HEAP_ZERO_MEMORY, (numTokens + 1) * sizeof(char *));
            if (newTokens == NULL)
            {
                Instance()->Win32.RtlFreeHeap(Instance()->Heap, HEAP_ZERO_MEMORY, tokens);

                return NULL;
            }
            for (int i = 0; i < numTokens - 1; i++)
            {
                newTokens[i] = tokens[i];
            }
            newTokens[numTokens - 1] = token;
            newTokens[numTokens] = NULL;
            Instance()->Win32.RtlFreeHeap(Instance()->Heap, HEAP_ZERO_MEMORY, tokens);

            tokens = newTokens;
        }

        // Move to the next token
        token = end;
    }

    Instance()->Win32.RtlFreeHeap(Instance()->Heap, HEAP_ZERO_MEMORY, stringCopy);

    return tokens;
}

FUNC INT MemCompare(PVOID s1, PVOID s2, INT len)
{
    PUCHAR p = s1;
    PUCHAR q = s2;
    INT charCompareStatus = 0;

    if (s1 == s2)
    {
        return charCompareStatus;
    }

    while (len > 0)
    {
        if (*p != *q)
        {
            charCompareStatus = (*p > *q) ? 1 : -1;
            break;
        }
        len--;
        p++;
        q++;
    }
    return charCompareStatus;
}

FUNC PVOID MmGadgetFind(
    _In_ PVOID Memory,
    _In_ SIZE_T Length,
    _In_ PVOID PatternBuffer,
    _In_ SIZE_T PatternLength)
{
    /* check if required arguments have been specified */
    if ((!Memory || !Length) ||
        (!PatternBuffer || !PatternLength))
    {
        return NULL;
    }

    /* now search for gadgets/pattern */
    for (SIZE_T Len = 0; Len < Length; Len++)
    {
        if (MemCompare(C_PTR(U_PTR(Memory) + Len), PatternBuffer, PatternLength) == 0)
        {
            return C_PTR(U_PTR(Memory) + Len);
        }
    }

    return NULL;
}

FUNC ULONG RandomNumber32(
    VOID)
{
    STARDUST_INSTANCE
    ULONG Seed = 0;

    Seed = NtGetTickCount();
    Seed = CustomRtlRandomEx(&Seed);
    Seed = CustomRtlRandomEx(&Seed);
    Seed = (Seed % (LONG_MAX - 2 + 1)) + 2;

    return Seed % 2 == 0 ? Seed : Seed + 1;
}

FUNC LARGE_INTEGER MillisecondsToLargeInteger(DWORD dwMilliseconds)
{
    LARGE_INTEGER liTimeout;
    liTimeout.QuadPart = -(LONGLONG)dwMilliseconds * 10000; // Negative for relative time
    return liTimeout;
}

FUNC INT StringToInt(LPSTR str)
{
    int result = 0;
    int sign = 1;

    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n')
    {
        str++;
    }

    // Check for sign
    if (*str == '-')
    {
        sign = -1;
        str++;
    }
    else if (*str == '+')
    {
        str++;
    }

    // Convert characters to integer
    while (*str >= '0' && *str <= '9')
    {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}

FUNC INT StringNCompareA(_In_ LPCSTR String1, _In_ LPCSTR String2, _In_ SIZE_T n)
{
    if (n == 0)
        return 0; // No characters to compare

    for (; n > 0 && *String1 == *String2; n--, String1++, String2++)
    {
        if (*String1 == '\0')
            return 0;
    }

    if (n == 0)
        return 0; // Equal up to the first n characters

    return ((*(LPCSTR)String1 < *(LPCSTR)String2) ? -1 : +1);
}

// from ReactOS
FUNC VOID CustomRtlInitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString)
{
    SIZE_T Size;
    SIZE_T MaxSize = 65534; // an even number

    if (SourceString)
    {
        Size = StringLengthW(SourceString) * sizeof(WCHAR);

        if (Size > MaxSize)
            Size = MaxSize;
        DestinationString->Length = (USHORT)Size;
        DestinationString->MaximumLength = (USHORT)Size + sizeof(UNICODE_NULL);
    }
    else
    {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
    }

    DestinationString->Buffer = (PWCHAR)SourceString;
}

FUNC ULONG CustomRtlRandomEx(PULONG Seed)
{
    ULONG RtlpRandomExConstantVector[128] =
        {
            0x4c8bc0aa, 0x51a0b326, 0x7112b3f1, 0x1b9ca4e1, /*   0 */
            0x735fc311, 0x6fe48580, 0x320a1743, 0x494045ca, /*   4 */
            0x103ad1c5, 0x4ba26e25, 0x62f1d304, 0x280d5677, /*   8 */
            0x070294ee, 0x7acef21a, 0x62a407d5, 0x2dd36af5, /*  12 */
            0x194f0f95, 0x1f21d7b4, 0x307cfd67, 0x66b9311e, /*  16 */
            0x60415a8a, 0x5b264785, 0x3c28b0e4, 0x08faded7, /*  20 */
            0x556175ce, 0x29c44179, 0x666f23c9, 0x65c057d8, /*  24 */
            0x72b97abc, 0x7c3be3d0, 0x478e1753, 0x3074449b, /*  28 */
            0x675ee842, 0x53f4c2de, 0x44d58949, 0x6426cf59, /*  32 */
            0x111e9c29, 0x3aba68b9, 0x242a3a09, 0x50ddb118, /*  36 */
            0x7f8bdafb, 0x089ebf23, 0x5c37d02a, 0x27db8ca6, /*  40 */
            0x0ab48f72, 0x34995a4e, 0x189e4bfa, 0x2c405c36, /*  44 */
            0x373927c1, 0x66c20c71, 0x5f991360, 0x67a38fa3, /*  48 */
            0x4edc56aa, 0x25a59126, 0x34b639f2, 0x1679b2ce, /*  52 */
            0x54f7ba7a, 0x319d28b5, 0x5155fa54, 0x769e6b87, /*  56 */
            0x323e04be, 0x4565a5aa, 0x5974b425, 0x5c56a104, /*  60 */
            0x25920c78, 0x362912dc, 0x7af3996f, 0x5feb9c87, /*  64 */
            0x618361bf, 0x433fbe97, 0x0244da8e, 0x54e3c739, /*  68 */
            0x33183689, 0x3533f398, 0x0d24eb7c, 0x06428590, /*  72 */
            0x09101613, 0x53ce5c5a, 0x47af2515, 0x2e003f35, /*  76 */
            0x15fb4ed5, 0x5e5925f4, 0x7f622ea7, 0x0bb6895f, /*  80 */
            0x2173cdb6, 0x0467bb41, 0x2c4d19f1, 0x364712e1, /*  84 */
            0x78b99911, 0x0a39a380, 0x3db8dd44, 0x6b4793b8, /*  88 */
            0x09b0091c, 0x47ef52b0, 0x293cdcb3, 0x707b9e7b, /*  92 */
            0x26d33ca3, 0x1e527faa, 0x3fe08625, 0x42560b04, /*  96 */
            0x139d2e78, 0x0b558cdb, 0x28a68b82, 0x7ba3a51d, /* 100 */
            0x52dabe9d, 0x59c3da1d, 0x5676cf9c, 0x152e972f, /* 104 */
            0x6d8ac746, 0x5eb33591, 0x78b30601, 0x0ab68db0, /* 108 */
            0x34737bb4, 0x1b6dd168, 0x76d9750b, 0x2ddc4ff2, /* 112 */
            0x18a610cd, 0x2bacc08c, 0x422db55f, 0x169b89b6, /* 116 */
            0x5274c742, 0x615535dd, 0x46ad005d, 0x4128f8dd, /* 120 */
            0x29f5875c, 0x62c6f3ef, 0x2b3be507, 0x4a8e003f  /* 124 */
        };

    ULONG RtlpRandomExAuxVarY = 0x7775fb16;

    ULONG Rand;
    int Pos;

    Pos = RtlpRandomExAuxVarY & (sizeof(RtlpRandomExConstantVector) / sizeof(RtlpRandomExConstantVector[0]) - 1);
    RtlpRandomExAuxVarY = RtlpRandomExConstantVector[Pos];
    Rand = (*Seed * LCG_A + LCG_C) % MAXLONG;
    RtlpRandomExConstantVector[Pos] = Rand;
    *Seed = Rand;

    return Rand;
}
