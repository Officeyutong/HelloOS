ORG 0xC600  ;引导器加载的后面的扇区的地址

; 0x7E00 ~ 0x7FFFF - 实模式可用内存
; 0x500 ~ 0x7BFF - 实模式可用内存

; 实模式栈放在 0x8000
; 扩展引导器放在 0x8100 ~ 0x84FF (1024byte)
; FAT表遍历缓存放在 0x8500 ~ 0x86FF (512Byte)

; VBE查询缓存放在0x8700 ~ 0x88FF(512B)

; 根目录表放在 0xA600~0xC5FF(512*16 byte)
; progload放在 0xC600 ~ 0xE5FF ; 最多8K
; VBE_INFO      0xE600~E6FF
; 内核缓存: 0xE700~...0x7FFFFF 0x71900 (最多454.25K)


; 保护模式内核
; 代码 - DATA - RODATA - BSS          
; 0x28 0000 ~ 0x37 ffff, 偏移0x0,0x80000,512KB 1M整, 256个4K页
; 栈            
; 0x38 0000 ~ 0x47 ffff, 地址偏移0xffff0
[BITS 16]
    CYLS_STORE EQU 0x0ff0; 读入柱面数的存储地址

    CODE_SEGMENT EQU 0x00280000 ; C程序代码段
    DISK_CACHE EQU 0x00100000; 磁盘数据缓存
    VBE_INFO EQU 0xE600 ; 显示信息
    VBE_QUERY_CACHE EQU 0x8700
    
    ROOT_DIR_STORE EQU 0xA600
    PROGLOADER_STORE EQU 0xC600 ; ProgLoader地址
    FAT_ACCESS_CACHE EQU 0x8500

    FUNC_DISPLAY_STRING EQU 0x500
    FUNC_READ_FILE EQU 0x500 + 2
    FUNC_FIND_FILE EQU 0x500 + 4

    VAR_MESSAGE_PREFIX EQU 0x600; 2byte
    VAR_KERNEL_SIZE EQU 0x600 + 2 ; 4byte
    VAR_LAST_SIZE EQU 0x600 + 2 + 4 ; 4byte
    VAR_LED_STATE_STORE EQU 0x600 + 10; 1byte 键盘LED灯状态的存储地址
    VAR_FAT_SIZE EQU 0x600 + 11; 4byte, FAT表长度
    VAR_FAT_START EQU 0x600 + 15; 2byte, FAT表起始位置
    VAR_ROOT_ENTRY_START EQU 0x600 + 17; 4byte, 根目录区起始位置
    VAR_ROOT_ENTRY_SIZE EQU 0x600 + 21; 2byte, 根目录区扇区数
    VAR_CLUSTER_SIZE_IN_SECTOR EQU 0x600 + 23;1byte 簇大小，扇区
    VAR_CLUSTER_SIZE_IN_BYTE EQU 0x600 + 24;4byte 簇大小，字节
    VAR_DATA_START_SECTOR EQU 0x600 + 28; 数据区开始扇区，4byte
    VAR_ROOT_DIR_ENTRIES EQU 0x600 + 32; 根目录区项数，4byte
    VAR_BOOT_DRIVE_NUMBER EQU 0x600 + 36; 启动驱动器号，1byte

    VAR_MEMORY_ENTRY_COUNT EQU 0x700 ; 内存项数, 4byte
    VAR_MEMORY_ENTRY_ARRAY EQU 0x700 + 4

    KERNEL_CACHE EQU 0xE700
    KERNEL_OFFSET EQU 0x280000

    MOV     SI, MESSAGE_PROGLOADER_LOADED
    CALL    display_string


    ; 内核程序整个读进来，塞到0x10000里

    MOV     SI, KERNEL_FILENAME ; 0xc106
    ; 簇号存在EDI里
    CALL    [FUNC_FIND_FILE] 
    CMP     EDI, 0xffffffff; 0xc10d
    JNE     kernel_loaded

    MOV     SI, MESSAGE_KERNEL_NOT_FOUND
    CALL    display_string
    JMP     $


kernel_loaded:
    MOV     EAX, [VAR_LAST_SIZE] ; 0xc11a
    MOV     [VAR_KERNEL_SIZE], EAX
    MOV     SI, DI
    MOV     AX, KERNEL_CACHE
    MOV     DI, AX
    MOV     AX, 0
    MOV     ES, AX

    ; PUSH    SI
    ; MOV     SI, MESSAGE_PROGLOADER_LOADED
    ; CALL    display_string
    ; POP     SI
    ; JMP $    ; 0xc62f

    CALL    [FUNC_READ_FILE] ; 0xc62f
    ; JMP $
    MOV     SI, MESSAGE_KERNEL_LOADED ; 0xc636
    CALL    display_string; 0xc639

    
    ; 查询VBE控制器信息 https://wiki.osdev.org/VESA
    MOV     AX, 0
    MOV     ES, AX
    MOV     AX, 0x4F00
    MOV     DI, VBE_QUERY_CACHE
    INT     0x10
    MOV     SI, MESSAGE_FAILED_TO_QUERY_VBE_INFO
    CMP     AX, 0x004F
    JNE     DIE ; 查询失败
    MOV     SI, MESSAGE_VBE_CONTROLLER_INFO_LOADED
    CALL    display_string


    MOV     WORD DS, [VBE_QUERY_CACHE + 14 + 2]
    MOV     WORD SI, [VBE_QUERY_CACHE + 14]
vbe_query_next_mode:

    MOV     AX, VBE_INFO / 16; 0xc132
    MOV     ES, AX
    MOV     DI, 0

    

    ; 获取VBE信息, 输出到ES:DI
    MOV     AX, 0x4F01 ;0x8208
    MOV     WORD CX, [DS:SI]; 显示模式ID 
    CMP     CX, 0xFFFF
    JE      vbe_query_no_suitable_mode
    INT     0x10 ; 0xc671

    CMP     WORD [VBE_INFO + 18], 1024; 宽度
    JNE     vbe_query_skip_curr
    CMP     WORD [VBE_INFO + 20], 768; 高度
    JNE     vbe_query_skip_curr
    CMP     BYTE [VBE_INFO + 25], 24 ; BPP
    JNE     vbe_query_skip_curr
    JMP     vbe_query_done
vbe_query_no_suitable_mode:
    MOV     SI, MESSAGE_NO_SUITABLE_MODE
    CALL    DIE
vbe_query_skip_curr:
    ADD     SI, 2
    JMP     vbe_query_next_mode
vbe_query_done:
    MOV     BX, [DS:SI]
    OR      BX, 0x4000
    ; SuperVGA视频模式
    MOV     AX, 0x4F02 ; 设置SuperVGA视频模式 0xc69e
    ; MOV     BX, 0x4118 ; 1024x768x16M 0x4000 | 0x118 (启用Linear FrameBuffer)
    INT     0x10
    
    MOV     SI, MESSAGE_VIDEO_MODE_LOADED
    CALL display_string ; 0xc6a6



    ; 探测内存
detect_memory_start:
    MOV     DWORD [VAR_MEMORY_ENTRY_COUNT], 0
    XOR     EBX, EBX
    MOV     DWORD EDI, VAR_MEMORY_ENTRY_ARRAY
    MOV     AX, 0
    MOV     ES, AX
detect_memory_continue:
    MOV     EDX, 0x534D4150 ; 0xc654
    MOV     EAX, 0xE820
    MOV     ECX, 24
    INT     0x15 ; 0xc6c9

    CMP     EBX, 0 ; 0xc668
    JE      detect_memory_done

    MOV     DWORD EAX, [EDI + 8]
    MOV     DWORD ECX, [EDI + 12]
    OR      EAX, ECX
    CMP     EAX, 0
    JE      detect_memory_continue ; 抛弃长度为0的

    INC     DWORD [VAR_MEMORY_ENTRY_COUNT] ; 0xc681
    ADD     EDI, 24
    JMP     detect_memory_continue
detect_memory_done:
    INC     DWORD [VAR_MEMORY_ENTRY_COUNT] ; 0xc6ef
    ; 获取指示灯状态
    MOV     AH, 0x02 ; 0xc14a
    INT     0x16 ; 0xc6f6
    MOV     BYTE [VAR_LED_STATE_STORE], AL

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

    XOR     AX, AX ; 0xc16a
    MOV     DS, AX
    LGDT    [GDTR] ;加载GDT表 0x824d

    MOV		EAX, CR0
    AND		EAX, 0x7fffffff	
    OR		EAX, 0x00000001	
    MOV		CR0, EAX ; 把CR0的bit0改成1，CR0具体有什么，见参考资料 0x825b
    
    JMP     DWORD   1<<3:flush_pipeline ; 0xc72d
[BITS 32]
flush_pipeline:
    ; 这里就到了保护模式了
    ; 设置栈
    MOV     ESP, 0x47fff0  ; 
    ; 刷新流水线
    MOV		AX, 2<<3		;  全局不可执行段
    MOV		DS, AX          ; 
    MOV		ES, AX
    MOV		FS, AX
    MOV		GS, AX
    MOV     AX, 2<<3        ; 保护模式栈
    MOV		SS, AX          

    ; 进行一个内核的拷贝
    MOV     ECX, [VAR_KERNEL_SIZE] ; 0xc1a2
    MOV     ESI, KERNEL_CACHE
    MOV     EDI, KERNEL_OFFSET
    CLD
    REP MOVSB ; 0xc75d


    
    ; 设置一会要用的数据段寄存器
    ; MOV     AX, 2<<3 ; C程序放的段，可执行; 0xc183
    ; MOV     DS, AX
    ; 获取主函数地址偏移
    ; MOV     ECX, [KERNEL_OFFSET + 0] ; 0xc1b5
    ; ADD     ECX, KERNEL_OFFSET  ; 内核主函数的绝对地址
    MOV     EAX, 1<<3; 全局可执行段 0xc75f
    PUSH    AX  ; 0xc1e4 
    PUSH    DWORD   KERNEL_OFFSET
    ; MOV     AX, 1<<3
    ; MOV     DS, AX
    JMP     FAR [SS:ESP]  ; 0xc76b

wait_for_keyword_out:
    ; 等待IOPort 0x64清空
    IN      AL, 0x64
    AND     AL, 0x02
    JNZ     wait_for_keyword_out
    RET
; memcpy4byte:
;     ; 以4byte为单位拷贝内存
;     ; ESI - 源地址; EDI - 目标地址; ECX - 传输长度
;     MOV		EAX, [ESI]
;     ADD		ESI, 4
;     MOV		[EDI], EAX
;     ADD		EDI, 4
;     SUB		ECX, 1 
;     JNZ		memcpy4byte
;     RET

    ALIGNB	16, DB 0x00 ;接下来的地址对齐到16字节
GDT_START:
    ; 这块是GDT表的实际数据
    TIMES	8 DB 0x00			; 第一项必须留空
    ; DW		0xffff,0x0000,0x9200,0x004f	; 32位不可执行段(实模式内存)，从0x0000 0000 到0x000f ffff
    ; DW		0x00ff,0x0000,0x9a28,0x00c0	; 32位可执行段+数据段(C程序用)，从0x0028 0000 到，0x0037 ffff 共0xff个4K页 
    DW      0xffff,0x0000,0x9a00,0x00cf ; 全局可执行段，用于访问全局
    DW      0xffff,0x0000,0x9200,0x00cf ; 全局不可执行段，用于访问全局
    ; DW      0xffff,0x0000,0x9238,0x004f ; 数据段，用来放栈，从0x38 0000 ~ 0x47 ffff，共0xfffff+1个字节
GDT_END:
GDTR:
    ; 这块是用来喂给LGDT指令的
    DW	    GDT_END - GDT_START - 1	; 表长度-1
    DD		GDT_START	; 表数据区

    ALIGNB	16, DB 0x00
[BITS 16]
display_string:
    PUSH    SI
    MOV     AX, PROGLOADER_MESSAGE_PREFIX
    MOV     SI, AX
    CALL    [FUNC_DISPLAY_STRING]
    POP     SI
    CALL    [FUNC_DISPLAY_STRING]
    RET

KERNEL_FILENAME:
    DB "KERNEL",0x20,0x20
    DB "BIN"
MESSAGE_KERNEL_NOT_FOUND:
    DB "kernel.bin not found!"
    DB 0x0D, 0x0A
    DB 0x00
PROGLOADER_MESSAGE_PREFIX:
    DB "progloader: ",0x00
MESSAGE_PROGLOADER_LOADED:
    DB "progloader loaded."
    DB 0x0D, 0x0A, 0x00
MESSAGE_VIDEO_MODE_LOADED:
    DB "Video mode loaded."
    DB 0x0D, 0x0A, 0x00
MESSAGE_FAILED_TO_QUERY_VBE_INFO:
    DB "Failed to query VBE info!"
    DB 0x0D, 0x0A, 0x00
MESSAGE_NO_SUITABLE_MODE:
    DB "NO 1024*768*16M mode found!"
    DB 0x0D, 0x0A, 0x00
MESSAGE_VBE_CONTROLLER_INFO_LOADED:
    DB "Succeeded to load VBE controller info!"
    DB 0x0D, 0x0A, 0x00
MESSAGE_KERNEL_LOADED:
    DB "Succeeded to read the kernel!"
    DB 0x0D, 0x0A, 0x00

DIE:
    CALL display_string
    JMP $

TIMES 512*2 - ($ - $$) - 4 DB 0x00
DB 0xCA, 0xFE, 0xBA, 0xBE