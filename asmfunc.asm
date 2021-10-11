GLOBAL io_halt
GLOBAL write_global_uint32
GLOBAL load_gdt, load_idt

io_halt:
    HLT
    RET
; io_out8:
;     MOV EDX, [ESP + 4]
;     MOV BYTE AL, [ESP + 8]
;     OUT DX, AL
;     RET

load_gdt: ; load_gdt(uint16 length, uint32 addr)
    PUSH EBP
    MOV EBP, ESP
    SUB ESP, 6
    MOV EAX, [EBP + 4 + 4] ; EAX = addr
    MOV [EBP - 4], EAX
    MOV DWORD EAX, [EBP + 4 + 8] ; EAX = length
    MOV [EBP - 6], AX
    LGDT [EBP - 6]
    ADD ESP, 6
    LEAVE
    RET
load_idt: ; load_idt(uint16 length, uint32 addr)
    PUSH EBP
    MOV EBP, ESP
    SUB ESP, 6
    MOV EAX, [EBP + 4 + 4] ; EAX = addr
    MOV [EBP - 4], EAX
    MOV DWORD EAX, [EBP + 4 + 8] ; EAX = length
    MOV [EBP - 6], AX
    LIDT [EBP - 6]
    ADD ESP, 6
    LEAVE
    RET
