GLOBAL load_gdt

load_gdt: ; load_gdt(uint16 length, uint32 addr)
    PUSH EBP
    MOV EBP, ESP
    SUB ESP, 6
    
    MOV WORD AX, [EBP + 4 + 4] ; EAX = length
    MOV WORD [EBP - 6], AX

    MOV DWORD EAX, [EBP + 4 + 8] ; EAX = addr
    MOV DWORD [EBP - 4], EAX
    
    LGDT [EBP - 6]
    ADD ESP, 6
    LEAVE
    RET