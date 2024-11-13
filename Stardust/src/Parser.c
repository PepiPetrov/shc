#include <PackageParser.h>

FUNC PCHAR ParserGetTaskID(PBUFFER Buffer)
{
    STARDUST_INSTANCE
    PCHAR Data = Instance()->Win32.RtlAllocateHeap(Instance()->Heap, HEAP_ZERO_MEMORY, 6);

    if (Buffer->Length < 6)
    {
        return NULL;
    }

    MmCopy(Data, (PCHAR)Buffer->Buffer, 6);

    Buffer->Length -= 6;
    Buffer->Buffer += 6;

    return Data;
}

FUNC PBYTE ParserGetBytes(PBUFFER Buffer, PUINT32 Size)
{
    UINT32 Length = 0;
    PBYTE outdata = NULL;

    if (!Buffer)
        return NULL;

    if (Buffer->Length < 4)
        return NULL;

    MmCopy(&Length, Buffer->Buffer, 4);
    Buffer->Buffer += 4;

    outdata = (PBYTE)Buffer->Buffer;
    if (outdata == NULL)
        return NULL;

    Buffer->Length -= 4;
    Buffer->Length -= Length;
    Buffer->Buffer += Length;

    if (Size != NULL)
        *Size = Length;

    return outdata;
}

FUNC INT ParserGetInt32(PBUFFER Buffer)
{
    INT32 intBytes = 0;

    if (!Buffer)
        return 0;

    if (Buffer->Length < 4)
        return 0;

    MmCopy(&intBytes, Buffer->Buffer, 4);

    Buffer->Buffer += 4;
    Buffer->Length -= 4;

    return (INT)intBytes;
}

FUNC PPACKAGE PackageCreate(PCHAR TaskID)
{
    STARDUST_INSTANCE
    PPACKAGE Package = NULL;

    Package = Instance()->Win32.RtlAllocateHeap(Instance()->Heap, HEAP_ZERO_MEMORY, sizeof(PACKAGE));
    Package->Buffer = Instance()->Win32.RtlAllocateHeap(Instance()->Heap, HEAP_ZERO_MEMORY, sizeof(BYTE));
    Package->Length = 0;
    Package->TaskID = TaskID;
    Package->Destroy = TRUE;

    return Package;
}

FUNC VOID Int32ToBuffer(
    OUT PUCHAR Buffer,
    IN UINT32 Size)
{
    (Buffer)[0] = (Size >> 24) & 0xFF;
    (Buffer)[1] = (Size >> 16) & 0xFF;
    (Buffer)[2] = (Size >> 8) & 0xFF;
    (Buffer)[3] = (Size) & 0xFF;
}

FUNC VOID PackageAddInt32(
    _Inout_ PPACKAGE Package,
    IN UINT32 Data)
{
    STARDUST_INSTANCE

    if (!Package)
    {
        return;
    }

    // If Package->Buffer is null, allocate it initially
    // Reallocate the buffer using HeapReAlloc
    Package->Buffer = Instance()->Win32.RtlReAllocateHeap(
        Instance()->Heap,                // Handle to the heap
        HEAP_ZERO_MEMORY,                // Options (can be 0 or use HEAP_ZERO_MEMORY)
        Package->Buffer,                 // The current buffer pointer
        Package->Length + sizeof(UINT32) // New size (old size + size of UINT32)
    );

    // Add the new int32 to the buffer
    Int32ToBuffer(Package->Buffer + Package->Length, Data);

    // Update the package length
    Package->Length += sizeof(UINT32);
}

FUNC VOID PackageAddBytes(PPACKAGE Package, PBYTE Data, SIZE_T Size)
{
    STARDUST_INSTANCE

    if (!Package)
    {
        return;
    }

    PackageAddInt32(Package, Size);

    if (Size)
    {
        Package->Buffer = Instance()->Win32.RtlReAllocateHeap(
            Instance()->Heap,      // Handle to the heap
            HEAP_ZERO_MEMORY,      // Options (can be 0 or use HEAP_ZERO_MEMORY)
            Package->Buffer,       // The current buffer pointer
            Package->Length + Size // New size (old size + size of string)
        );

        MmCopy(Package->Buffer + Package->Length, Data, Size);

        Package->Length += Size;
    }
}

FUNC VOID PackageAddString(PPACKAGE package, PCHAR data)
{
    PackageAddBytes(package, (PBYTE)data, StringLengthA(data));
}

FUNC VOID PackageDestroy(
    IN PPACKAGE Package)
{
    STARDUST_INSTANCE

    if (Package)
    {

        if (Package->Buffer)
        {
            MmSet(Package->Buffer, 0, Package->Length);
            Instance()->Win32.RtlFreeHeap(Instance()->Heap, HEAP_ZERO_MEMORY, Package->Buffer);
            Package->Buffer = NULL;
        }

        MmSet((PVOID)Package, 0, sizeof(PACKAGE));
        Instance()->Win32.RtlFreeHeap(Instance()->Heap, HEAP_ZERO_MEMORY, Package);
        Package = NULL;
    }
}
