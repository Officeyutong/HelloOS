EXTERN c_interrupt_0x21
EXTERN c_interrupt_0x27
EXTERN c_interrupt_0x2c

GLOBAL asm_interrupt_0x21
GLOBAL asm_interrupt_0x27
GLOBAL asm_interrupt_0x2c



asm_interrupt_0x21:
    ; 保存执行上下文
    PUSH    ES
    PUSH    DS
    PUSHAD
    MOV     EAX, ESP
    PUSH    EAX ; 函数传参
    MOV     AX, SS
    MOV     ES, AX
    MOV     DS, AX
    CALL    c_interrupt_0x21
    POP     EAX
    POPAD
    POP     DS
    POP     ES
    IRETD

asm_interrupt_0x27:
    ; 保存执行上下文
    PUSH    ES
    PUSH    DS
    PUSHAD
    MOV     EAX, ESP
    PUSH    EAX ; 函数传参
    MOV     AX, SS
    MOV     ES, AX
    MOV     DS, AX
    CALL    c_interrupt_0x27
    POP     EAX
    POPAD
    POP     DS
    POP     ES
    IRETD

asm_interrupt_0x2c:
    ; 保存执行上下文
    PUSH    ES
    PUSH    DS
    PUSHAD
    MOV     EAX, ESP
    PUSH    EAX ; 函数传参
    MOV     AX, SS
    MOV     ES, AX
    MOV     DS, AX
    CALL    c_interrupt_0x2c
    POP     EAX
    POPAD
    POP     DS
    POP     ES
    IRETD