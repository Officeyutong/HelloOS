ORG 0x8200  ;引导器加载的后面的扇区的地址

; 0x7E00 ~ 0x7FFFF - 实模式可用内存
; 0x500 ~ 0x7BFF - 实模式可用内存

[BITS 16]
    CYLS_STORE EQU 0x0ff0; 读入柱面数的存储地址
    LED_STATE_STORE EQU 0x0ff1; 键盘LED灯状态的存储地址
    VMODE_STATE_STORE EQU 0x0ff2; VMODE状态
    ; SCREEN_X_STORE EQU 0x0ff4; 分辨率X
    ; SCREEN_Y_STORE EQU 0x0ff6; 分辨率Y
    ; VRAM EQU 0x0ff8; 图像缓冲区开始地址

    ; VBE_INFO EQU 0x1000 ;/ VBE 信息结构体(512 byte)

    CODE_SEGMENT EQU 0x00280000 ; C程序代码段
    DISK_CACHE EQU 0x00100000; 磁盘数据缓存
    VBE_INFO EQU 0x9000

    MOV AX, VBE_INFO / 16; 
    MOV ES, AX
    MOV DI, 0

    ; 获取VBE信息, 输出到ES:DI
    MOV AX, 0x4F01 ;0x8208
    MOV CX, 0x0118
    INT 0x10

   ; SuperVGA视频模式
    MOV AX, 0x4F02 ; 设置SuperVGA视频模式 
    MOV BX, 0x4118 ; 1024x768x16M 0x4000 | 0x118 (启用Linear FrameBuffer)
    INT 0x10
    
    ; 拷贝512字节的结构体到VBE_INFO处
    ; MOV AX, ES
    ; MOV DS, AX
    ; MOV AX, DI
    ; MOV SI, AX
    ; MOV AX, VBE_INFO / 16 ; 0x8218
    ; MOV DS, AX
    ; MOV SI, 0

    ; MOV AX, [VBE_INFO]
    ; SHR AX, 4 ;
    ; MOV ES, AX
    ; XOR DI, DI ; 清理出来ES:DI的地址
    
    ; MOV CX, 512
    ; CLD ; 0x822d
    ; REP MOVSB ; 拷贝512字节的结构体 

    ;获取指示灯状态
    ; MOV AH, 0x02
    ; INT 0x16
    ; MOV BYTE [LED_STATE_STORE], AL


    MOV		AL, 0xff
    OUT		0x21, AL; 0x21是第一块PIC主寄存器的数据端口
    NOP						
    OUT		0xa1, AL; 0xa1是第一块PIC主寄存器的数据端口
    CLI				; 禁用BIOS中断

    CALL	wait_for_keyword_out ; 根据键盘控制器的设计，如果我们要往0x64或者0x60写东西，那么必须等着缓冲区清空
    MOV		AL, 0xd1 ; 0x823b
    OUT		0x64, AL ; 发送控制指令0xd1，表示把0x60写入的下一个字节扔到输出端口
    CALL	wait_for_keyword_out
    MOV		AL, 0xdf ; 0x8242		
    OUT		0x60, AL ; 把0xdf写到输出端口，这其中(0xdf) & (0x02) == 1即打开A20 gate
    CALL	wait_for_keyword_out

    XOR     AX, AX
    MOV     DS, AX
    LGDT [GDTR] ;加载GDT表 0x824d

    MOV		EAX, CR0
    AND		EAX, 0x7fffffff	
    OR		EAX, 0x00000001	
    MOV		CR0, EAX ; 把CR0的bit0改成1，CR0具体有什么，见参考资料 0x825b
    
    JMP DWORD 3<<3:flush_pipeline ; 0x8262
[BITS 32]
flush_pipeline:
    ; 设置栈
    MOV     ESP, 0x000ffff0 
    ; 刷新流水线
    MOV		AX, 1<<3		;  可读写不可执行32位数据段
    MOV		DS, AX          ; 
    MOV		ES, AX
    MOV		FS, AX
    MOV		GS, AX
    MOV     AX, 5<<3
    MOV		SS, AX          

    ; 拷贝C程序数据到代码段
    MOV     AX, 4<<3        ; 全局可读写段
    MOV     DS, AX          ; 
    MOV		ESI, cprog
    MOV		EDI, CODE_SEGMENT ; CODE_SEGMENT = 0x280000
    MOV		ECX, 512*1024/4 ; 0x8291
    CALL	memcpy4byte
    ; 拷贝bootloader
    MOV     ESI, 0x7c00
    MOV     EDI, DISK_CACHE
    MOV     ECX, 512/4 ; bootloader有512个字节
    CALL    memcpy4byte
    ; 拷贝除了bootloader之外的扇区
    MOV     ESI, 0x8200
    MOV     EDI, DISK_CACHE + 512
    XOR     ECX, ECX    ; 0x82b6
    MOV     CL, BYTE [CYLS_STORE]
    IMUL    ECX, 18 * 2 ; 一共18 * 2 * [CYLS_STORE]个扇区
    SUB     ECX, 1; 扣掉启动扇区
    SHR     ECX, 2; 除掉4
    CALL    memcpy4byte

    
    ; 设置一会要用的数据段寄存器
    MOV     AX, 2<<3 ; C程序放的段，可执行; 0x82cf
    MOV     DS, AX
    ; 获取主函数地址偏移
    MOV     ECX, [DS:0]
    ADD     ECX, 4  ; 地址偏移不包括前四个字节
    MOV     EAX, 2<<3
    PUSH    AX  ; 0x82e4 
    PUSH    CX
    MOV     AX, 1<<3
    MOV     DS, AX
    ; JMP FAR [SS:ESP]  ; 0x82e8
    JMP DWORD 2<<3:0x36

wait_for_keyword_out:
    ; 等待IOPort 0x64清空
    IN AL, 0x64
    AND AL, 0x02
    JNZ wait_for_keyword_out
    RET
memcpy4byte:
    ; 以4byte为单位拷贝内存
    ; ESI - 源地址; EDI - 目标地址; ECX - 传输长度
    MOV		EAX, [ESI]
    ADD		ESI, 4
    MOV		[EDI], EAX
    ADD		EDI, 4
    SUB		ECX, 1 
    JNZ		memcpy4byte
    RET

    ALIGNB	16, DB 0x00 ;接下来的地址对齐到16字节
GDT_START:
    ; 这块是GDT表的实际数据
    TIMES	8 DB 0x00			; 第一项必须留空
    DW		0xffff,0x0000,0x9200,0x004f	; 32位不可执行段，从0x0000 0000 到0x000f ffff
    DW		0xffff,0x0000,0x9a28,0x0047	; 32位可执行段(C程序用)，从0x0028 0000 到，0x002f ffff 共0x7ffff个字节 
    DW      0xffff,0x0000,0x9a00,0x00cf ; 全局可执行段，用于访问全局
    DW      0xffff,0x0000,0x9200,0x00cf ; 全局不可执行段，用于访问全局
    DW      0xffff,0x0000,0x9238,0x004f ; 数据段，用来放栈，从0x38 0000 ~ 0x47 ffff，共0xfffff+1个字节
GDT_END:
GDTR:
    ; 这块是用来喂给LGDT指令的
    DW		6*8 - 1	; 表长度-1
    DD		GDT_START	; 表数据区

    ALIGNB	16, DB 0x00

TIMES 512 - ($ - $$) DB 0x00
cprog: ;从第三个扇区开始是C程序了