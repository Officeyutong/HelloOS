EXTERN c_interrupt_0x0e
EXTERN c_interrupt_0x21
EXTERN c_interrupt_0x27
EXTERN c_interrupt_0x2c
EXTERN kernel_page_directory


GLOBAL asm_interrupt_0x0e
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

asm_interrupt_0x0e: ; 页错误 
    ; 保存执行上下文
    PUSH    ES
    PUSH    DS
    PUSHAD ; EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI
    MOV     EAX, [ESP + 10*4]; 错误码
    PUSH    EAX 
    MOV     EAX, CR3 ; 发生错误的页目录
    PUSH    EAX 
    MOV     EAX, CR2 ; 试图访问的虚拟地址
    PUSH    EAX 
    ; 换成内核的页目录
    MOV     EAX, [kernel_page_directory]
    MOV     CR3, EAX
    MOV     AX, SS
    MOV     ES, AX
    MOV     DS, AX
    CALL    c_interrupt_0x0e
    POP     EAX ; 参数给他弹了
    POP     EAX
    MOV     CR3, EAX ; 换回旧的页目录
    POP     EAX
    POPAD
    POP     DS
    POP     ES
    ADD     ESP, 4; 去掉错误码
    IRETD