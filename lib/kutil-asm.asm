GLOBAL io_out8
GLOBAL io_out16
GLOBAL io_in8
GLOBAL io_in16
GLOBAL io_ins16

io_out8: ; void io_out8(uint16_t port,uint8_t data)
    PUSH EBP
    MOV EBP, ESP
    MOV BYTE AL, [EBP + 12] ; data
    MOV WORD DX, [EBP + 8] ; port
    OUT DX, AL
    LEAVE
    RET    

io_out16: ; void io_out16(uint16_t port,uint16_t data)
    PUSH EBP
    MOV EBP, ESP
    MOV WORD AX, [EBP + 12] ; data
    MOV WORD DX, [EBP + 8]
    OUT DX, AL
    LEAVE
    RET    

io_in8: ; uint8_t io_in8(uint16_t port)
    PUSH EBP
    MOV EBP, ESP
    MOV WORD DX, [ESP + 8]; port
    IN AL, DX ; ret = AL
    LEAVE
    RET

io_in16: ; uint16_t io_in16(uint16_t port)
    PUSH EBP
    MOV EBP, ESP
    MOV WORD DX, [ESP + 8]; port
    IN AX, DX ; ret = AX
    LEAVE
    RET

io_ins16: ; void io_ins16(uint16_t port, uint32_t buffer, uint32_t count)
    PUSH EBP
    MOV EBP, ESP
    PUSH EDI
    MOV WORD DX, [EBP + 8]
    MOV DWORD EDI, [EBP + 12]
    MOV DWORD ECX, [EBP + 16]
    CLD
    REP INSW
    POP EDI
    LEAVE
    RET

