#ifndef STARDUST_LDR_H
#define STARDUST_LDR_H

#include <Common.h>

PVOID LdrModulePeb(
    _In_ ULONG Hash);

PVOID LdrFunction(
    _In_ PVOID Module,
    _In_ ULONG Function);

PVOID LdrFunctionString(PVOID Module, PCHAR Function);

PVOID LdrModuleLoad(
    LPSTR ModuleName);

PIMAGE_NT_HEADERS LdrpImageHeader(
    _In_ PVOID Image);

#define LOAD_FUNCTION(Module, FunctionName)                                                                   \
    if (!(Instance()->Win32.FunctionName = LdrFunction(Instance()->Modules.Module, HASH_STR(#FunctionName)))) \
    {                                                                                                         \
        return;                                                                                               \
    }

#endif // STARDUST_LDR_H
