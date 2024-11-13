
#include <BeaconApi.h>
#include <stdio.h>

// the rest was taken from https://github.com/trustedsec/COFFLoader/blob/main/beacon_compatibility.c. credit goes to them
FUNC UINT32 swap_endianess(UINT32 indata)
{
    UINT32 testint = 0xaabbccdd;
    UINT32 outint = indata;

    if (((unsigned char *)&testint)[0] == 0xdd)
    {
        ((unsigned char *)&outint)[0] = ((unsigned char *)&indata)[3];
        ((unsigned char *)&outint)[1] = ((unsigned char *)&indata)[2];
        ((unsigned char *)&outint)[2] = ((unsigned char *)&indata)[1];
        ((unsigned char *)&outint)[3] = ((unsigned char *)&indata)[0];
    }
    return outint;
}

FUNC void BeaconDataParse(datap *parser, char *buffer, int size)
{
    if (parser == NULL)
    {
        return;
    }
    parser->original = buffer;
    parser->buffer = buffer;
    parser->length = size - 4;
    parser->size = size - 4;
    parser->buffer += 4;
    return;
}

FUNC int BeaconDataInt(datap *parser)
{
    UINT32 fourbyteint = 0;
    if (parser->length < 4)
    {
        return 0;
    }
    MmCopy(&fourbyteint, parser->buffer, 4);
    parser->buffer += 4;
    parser->length -= 4;
    return (int)fourbyteint;
}

FUNC short BeaconDataShort(datap *parser)
{
    UINT16 retvalue = 0;
    if (parser->length < 2)
    {
        return 0;
    }
    MmCopy(&retvalue, parser->buffer, 2);
    parser->buffer += 2;
    parser->length -= 2;
    return (short)retvalue;
}

FUNC int BeaconDataLength(datap *parser)
{
    return parser->length;
}

FUNC char *BeaconDataExtract(datap *parser, int *size)
{
    UINT32 length = 0;
    char *outdata = NULL;
    /*Length prefixed binary blob, going to assume uINT32 for this.*/
    if (parser->length < 4)
    {
        return NULL;
    }
    MmCopy(&length, parser->buffer, 4);
    parser->buffer += 4;

    outdata = parser->buffer;
    if (outdata == NULL)
    {
        return NULL;
    }
    parser->length -= 4;
    parser->length -= length;
    parser->buffer += length;
    if (size != NULL && outdata != NULL)
    {
        *size = length;
    }
    return outdata;
}

/* format API */

FUNC void BeaconFormatAlloc(formatp *format, int maxsz)
{
    STARDUST_INSTANCE

    if (format == NULL)
    {
        return;
    }
    format->original = Instance()->Win32.RtlAllocateHeap(Instance()->Heap, HEAP_ZERO_MEMORY, maxsz);
    format->buffer = format->original;
    format->length = 0;
    format->size = maxsz;
    return;
}

FUNC void BeaconFormatReset(formatp *format)
{
    MmZero(format->original, format->size);
    format->buffer = format->original;
    format->length = format->size;
    return;
}

FUNC void BeaconFormatFree(formatp *format)
{
    if (format == NULL)
    {
        return;
    }
    if (format->original)
    {
        format->original = NULL;
    }
    format->buffer = NULL;
    format->length = 0;
    format->size = 0;
    return;
}

FUNC void BeaconFormatAppend(formatp *format, char *text, int len)
{
    MmCopy(format->buffer, text, len);
    format->buffer += len;
    format->length += len;
    return;
}

FUNC void BeaconFormatPrintf(formatp *format, char *fmt, ...)
{
    STARDUST_INSTANCE

    /*Take format string, and sprintf it into here*/
    va_list args;
    int length = 0;

    va_start(args, fmt);
    length = Instance()->Win32.vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (format->length + length > format->size)
    {
        return;
    }

    va_start(args, fmt);
    (void)Instance()->Win32.vsnprintf(format->buffer, length, fmt, args);
    va_end(args);
    format->length += length;
    format->buffer += length;
    return;
}

FUNC char *BeaconFormatToString(formatp *format, int *size)
{
    *size = format->length;
    return format->original;
}

FUNC void BeaconFormatInt(formatp *format, int value)
{
    UINT32 indata = value;
    UINT32 outdata = 0;
    if (format->length + 4 > format->size)
    {
        return;
    }
    outdata = swap_endianess(indata);
    MmCopy(format->buffer, &outdata, 4);
    format->length += 4;
    format->buffer += 4;
    return;
}

/* Main output functions */

FUNC void BeaconPrintf(int type, char *fmt, ...)
{
    STARDUST_INSTANCE

    /* Change to maintain internal buffer, and return after done running. */
    int length = 0;
    char *tempptr = NULL;
    va_list args;
    va_start(args, fmt);
    Instance()->Win32.vprintf(fmt, args);
    va_end(args);

    va_start(args, fmt);
    length = Instance()->Win32.vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    tempptr = CustomReAlloc(Instance()->BOFOutput.beacon_compatibility_output, Instance()->BOFOutput.beacon_compatibility_size + length + 1);
    if (tempptr == NULL)
    {
        return;
    }
    Instance()->BOFOutput.beacon_compatibility_output = tempptr;
    MmZero(Instance()->BOFOutput.beacon_compatibility_output + Instance()->BOFOutput.beacon_compatibility_offset, length + 1);
    va_start(args, fmt);
    length = Instance()->Win32.vsnprintf(Instance()->BOFOutput.beacon_compatibility_output + Instance()->BOFOutput.beacon_compatibility_offset, length + 1, fmt, args);
    Instance()->BOFOutput.beacon_compatibility_size += length;
    Instance()->BOFOutput.beacon_compatibility_offset += length;
    va_end(args);
    return;
}

FUNC void BeaconOutput(int type, char *data, int len)
{
    STARDUST_INSTANCE

    Instance()->BOFOutput.beacon_compatibility_output = CustomReAlloc(Instance()->BOFOutput.beacon_compatibility_output, Instance()->BOFOutput.beacon_compatibility_size + len + 1);
    if (Instance()->BOFOutput.beacon_compatibility_output == NULL)
    {
        return;
    }
    MmZero(Instance()->BOFOutput.beacon_compatibility_output + Instance()->BOFOutput.beacon_compatibility_offset, len + 1);
    MmCopy(Instance()->BOFOutput.beacon_compatibility_output + Instance()->BOFOutput.beacon_compatibility_offset, data, len);
    Instance()->BOFOutput.beacon_compatibility_size += len;
    Instance()->BOFOutput.beacon_compatibility_offset += len;
    return;
}

/* Token Functions */

FUNC BOOL BeaconUseToken(HANDLE token)
{
    /* Leaving this to be implemented by people needing/wanting it */
    return TRUE;
}

FUNC void BeaconRevertToken(void)
{
    /* Leaving this to be implemented by people needing/wanting it */
    return;
}

FUNC BOOL BeaconIsAdmin(void)
{
    /* Leaving this to be implemented by people needing it */
    return FALSE;
}

/* Injection/spawning related stuffs
 *
 * These functions are basic place holders, and if implemented into something
 * real should be just calling internal functions for your tools. */
FUNC void BeaconGetSpawnTo(BOOL x86, char *buffer, int length)
{
    /* Leaving this to be implemented by people needing/wanting it */
    return;
}

FUNC BOOL BeaconSpawnTemporaryProcess(BOOL x86, BOOL ignoreToken, STARTUPINFO *sInfo, PROCESS_INFORMATION *pInfo)
{
    /* Leaving this to be implemented by people needing/wanting it */
    return FALSE;
}

FUNC void BeaconInjectProcess(HANDLE hProc, int pid, char *payload, int p_len, int p_offset, char *arg, int a_len)
{
    /* Leaving this to be implemented by people needing/wanting it */
    return;
}

FUNC void BeaconInjectTemporaryProcess(PROCESS_INFORMATION *pInfo, char *payload, int p_len, int p_offset, char *arg, int a_len)
{
    /* Leaving this to be implemented by people needing/wanting it */
    return;
}

FUNC void BeaconCleanupProcess(PROCESS_INFORMATION *pInfo)
{
    (void)SysNtClose(pInfo->hThread);
    (void)SysNtClose(pInfo->hProcess);
    return;
}

FUNC BOOL toWideChar(char *src, wchar_t *dst, int max)
{
    /* Leaving this to be implemented by people needing/wanting it */
    return FALSE;
}

FUNC char *BeaconGetOutputData(int *outsize)
{
    STARDUST_INSTANCE

    char *outdata = Instance()->BOFOutput.beacon_compatibility_output;

    if (outsize)
        *outsize = Instance()->BOFOutput.beacon_compatibility_size;

    Instance()->BOFOutput.beacon_compatibility_output = NULL;
    Instance()->BOFOutput.beacon_compatibility_size = 0;
    Instance()->BOFOutput.beacon_compatibility_offset = 0;

    return outdata;
}
