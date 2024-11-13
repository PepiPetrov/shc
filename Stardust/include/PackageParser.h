#ifndef STARDUST_PARSER_H
#define STARDUST_PARSER_H

#include <Common.h>

PCHAR ParserGetTaskID(PBUFFER Buffer);
PBYTE ParserGetBytes(PBUFFER Buffer, PUINT32 Size);
INT ParserGetInt32(PBUFFER Buffer);
PPACKAGE PackageCreate(PCHAR TaskID);
VOID Int32ToBuffer(
    OUT PUCHAR Buffer,
    IN UINT32 Size);
VOID PackageAddInt32(
    _Inout_ PPACKAGE Package,
    IN UINT32 Data);
VOID PackageAddBytes(PPACKAGE Package, PBYTE Data, SIZE_T Size);
VOID PackageAddString(PPACKAGE package, PCHAR data);
VOID PackageDestroy(
    IN PPACKAGE Package);

#endif
