#ifndef STARDUST_UTILS_H
#define STARDUST_UTILS_H

#include <Common.h>

ULONG HashString(
    _In_ PVOID  String,
    _In_ SIZE_T Length
);

PVOID GetInstance();

#endif //STARDUST_UTILS_H
