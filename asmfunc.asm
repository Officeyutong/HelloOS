GLOBAL io_halt,write_global_uint32

io_halt:
    HLT
    RET
; io_out8:
;     MOV EDX, [ESP + 4]
;     MOV BYTE AL, [ESP + 8]
;     OUT DX, AL
;     RET
write_global_uint32:
    PUSH EAX
    PUSH EBX
    PUSH ECX
    PUSH EDX
    MOV DWORD   EAX, [ESP + 4] ; 要写的值
    MOV DWORD   EBX, [ESP] ; 地址
    MOV WORD    ECX, DS
    MOV WORD    EDX, 4<<3
    MOV WORD    DS, DX
    MOV DWORD   [DS:EBX], EAX
    MOV WORD    DS, ECX
    POP EDX
    POP ECX
    POP EBX
    POP EAX
    RET
