#include <windows.h>
#include <stdio.h>

void FindForwardedFunctions(const char* dllPath) {
    // Load the DLL into memory
    HMODULE hModule = LoadLibraryExA(dllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (!hModule) {
        printf("Failed to load the DLL: %s\n", dllPath);
        return;
    }

    // Get the address of the DOS header
    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)hModule;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        printf("Invalid DOS signature\n");
        FreeLibrary(hModule);
        return;
    }

    // Get the address of the NT header
    IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)((BYTE*)hModule + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        printf("Invalid NT signature\n");
        FreeLibrary(hModule);
        return;
    }

    // Get the export directory
    IMAGE_EXPORT_DIRECTORY* exportDir = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)hModule + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    DWORD* nameRvas = (DWORD*)((BYTE*)hModule + exportDir->AddressOfNames);
    DWORD* funcRvas = (DWORD*)((BYTE*)hModule + exportDir->AddressOfFunctions);
    WORD* ordinals = (WORD*)((BYTE*)hModule + exportDir->AddressOfNameOrdinals);

    // Iterate over each exported function
    printf("Checking for forwarded functions in %s...\n", dllPath);
    for (DWORD i = 0; i < exportDir->NumberOfNames; ++i) {
        const char* funcName = (const char*)((BYTE*)hModule + nameRvas[i]);
        DWORD funcRva = funcRvas[ordinals[i]];
        const char* forwardStr = (const char*)((BYTE*)hModule + funcRva);

        // Check if the function is forwarded (pointing to another DLL)
        if ((funcRva >= ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress) &&
            (funcRva < ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)) {
            printf("Forwarded Function: %s -> %s\n", funcName, forwardStr);
        }
    }

    FreeLibrary(hModule);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <dll_path>\n", argv[0]);
        return 1;
    }

    const char* dllPath = argv[1];
    FindForwardedFunctions(dllPath);

    return 0;
}
