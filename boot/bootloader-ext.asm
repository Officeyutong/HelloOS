ORG 0x8100

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

    ; 0x7E00 ~ 0x7FFFF - 实模式可用内存
    ; 0x500 ~ 0x7BFF - 实模式可用内存


    ; 实模式栈放在 0x8000
    ; 扩展引导器放在 0x8100 ~ 0x84FF (1024byte)
    ; FAT表遍历缓存放在 0x8500 ~ 0x86FF (512Byte)

    ; 根目录表放在 0xA600~0xC5FF(512*16 byte)
    ; progload放在 0xC600 ~ 0xE5FF ; 最多8K


    MOV WORD [FUNC_READ_FILE], read_file
    MOV WORD [FUNC_FIND_FILE], find_file

    JMP boot_continue

    ALIGNB 4, DB 0x00
LBA_READ_STRUCT: ; 0x8108
    DB 16
    DB 0
BLOCK_COUNT:
    DW 0 ; 读取的块个数
DEST_MEMORY_ADDR:
    DW 0 ; 段偏移
    DW 0 ; 段地址
LBA:
    DD 0 ; 逻辑地址的低32位
    DD 0 ; 逻辑地址的高16位
boot_continue:
    MOV SI, MESSAGE_EXTBOOTLOADER_LOADED
    CALL [FUNC_DISPLAY_STRING]

    ; 读取根目录表
    MOV AX, [VAR_ROOT_ENTRY_SIZE]
    MOV WORD [BLOCK_COUNT], AX
    MOV WORD [DEST_MEMORY_ADDR], ROOT_DIR_STORE
    MOV EAX, [VAR_ROOT_ENTRY_START]
    MOV DWORD [LBA], EAX ; 引导扇区是0

    MOV AX, 0
    MOV DS, AX
    MOV SI, LBA_READ_STRUCT

    MOV AH, 0x42
    MOV DL, 0x80
    INT 0x13
    
    CMP AH, 0 ; 0x7ca2
    JE succeed_read_root_entry
    
    MOV SI, MESSAGE_FAILED_TO_LOAD_ROOT_ENTRY
    CALL [FUNC_DISPLAY_STRING]
    JMP $

succeed_read_root_entry:

    MOV SI, PROGLOAD_FILENAME
    CALL find_file
    CMP EDI, 0xFFFFFFFF

    JNE file_found
    MOV SI, PROGLOAD_NOT_FOUND
    CALL [FUNC_DISPLAY_STRING]
    JMP $
file_found:

    MOV ESI, EDI
    MOV AX, 0
    MOV ES, AX
    MOV DI, PROGLOADER_STORE
    CALL read_file; 0x816f
    JMP PROGLOADER_STORE

; 从根目录寻找文件
; SI 文件名地址, 2 byte
; OUT EDI: 簇号, 4byte (0xFFFF FFFF)表示未找到
find_file:
    MOV EAX, ROOT_DIR_STORE ; 根目录表 0x8166
    ; MOV BP, SP
    PUSH SI
    SUB SP, 2
    MOV WORD [ESP], 0; 循环计数器
scan:
    INC WORD [ESP]
    CMP BYTE [EAX + 11], 0x20 
    JNE scan_continue; 非文件，扫下一个

    MOV EBX, [EAX + 28]
    MOV [VAR_LAST_SIZE], EBX

    MOV BX, 0
    MOV ES, BX
    MOV DS, BX
    ; 文件名串
    MOV DI, AX
    ; 目标串
    MOV WORD BX, [ESP + 2]
    MOV SI, BX
    ; 串长
    MOV CX, 11
    ; JMP $
    CLD
    REP CMPSB
    JCXZ found_that
scan_continue:
    MOV DWORD EBX, [VAR_ROOT_DIR_ENTRIES]
    CMP WORD [ESP], BX
    JE not_found
    ADD DWORD EAX, 32
    JMP scan
not_found:
    MOV EDI, 0xFFFFFFFF
    JMP scan_exit
found_that:
    ; 记录下目录项簇号
    ; 低2字节
    MOV WORD DI, [EAX + 26]
    ; 高2字节
    MOV WORD BX, [EAX + 20]
    SHL EBX, 16
    ADD EDI, EBX
scan_exit:
    ADD ESP, 4
    RET


; 读取文件，保存到指定地址
;       ESI: 簇号, 4 byte
;       ES:DI: 缓冲地址
read_file: ; 0x81de
    PUSH DI
    PUSH ESI
    XOR ECX, ECX; 0x7d25
read_file_read_next_cluster:
    MOV DWORD EBX, [ESP]  ; 0x81e4
    ; 当前簇ID在[ESP]
    MOV ESI, EBX
    CALL read_cluster

    ; 计算所属的扇区
    SHR EBX, 7 ; 每个扇区存128个簇号，故右移7位

    MOV WORD [BLOCK_COUNT], 1 ; 一个扇区
    ; MOV AX, [ESP + 2] ; 目标地址
    MOV WORD [DEST_MEMORY_ADDR], FAT_ACCESS_CACHE
    MOV WORD [DEST_MEMORY_ADDR + 2], 0
    
    ; 计算扇区偏移
    MOV WORD CX, [VAR_FAT_START]
    ADD DWORD ECX, EBX
    MOV DWORD [LBA], ECX
    MOV AX, 0
    MOV DS, AX
    MOV SI, LBA_READ_STRUCT

    MOV AH, 0x42
    MOV DL, 0x80 ; 0x7d4e
    INT 0x13 
    MOV DWORD EAX, [ESP]
    ; 扇区内偏移
    AND EAX, 0x7f
    MOV DWORD EBX, [FAT_ACCESS_CACHE + EAX * 4] ; EBX: 下一个簇号

    CMP EBX, 0x0FFFFFF7 ; 0x7d71
    JGE read_file_end ; 读到头了
    
    
    MOV [ESP], EBX ; 保存下一个簇号
    MOV DWORD EBX, [VAR_CLUSTER_SIZE_IN_BYTE]
    SHR EBX, 4
    MOV AX, ES
    ADD AX, BX
    MOV ES, AX
    JMP read_file_read_next_cluster
read_file_end:
    ADD ESP, 6
    RET


; ESI: 簇号, ES:DI: 输出地址
read_cluster:
    PUSH EAX
    PUSH ESI
    XOR EAX, EAX
    ; 读一个簇进来
    MOV BYTE AL, [VAR_CLUSTER_SIZE_IN_SECTOR]
    SUB ESI, 3
    IMUL ESI, EAX
    ADD ESI, [VAR_DATA_START_SECTOR]

    MOV WORD [BLOCK_COUNT], AX ; 簇大小
    MOV AX, DI
    MOV WORD [DEST_MEMORY_ADDR], AX
    MOV AX, ES
    MOV WORD [DEST_MEMORY_ADDR + 2], AX
    MOV DWORD [LBA], ESI
    MOV AX, 0
    MOV DS, AX
    MOV SI, LBA_READ_STRUCT

    MOV AH, 0x42
    MOV DL, 0x80
    INT 0x13
    POP ESI
    POP EAX
    RET


MESSAGE_EXTBOOTLOADER_LOADED:
    DB "Ext Bootloader loaded"
    DB 0x0D,0x0A
    DB 0x00
PROGLOAD_NOT_FOUND:
    DB "PROGLOAD.BIN not found!"
    DB 0x0D,0x0A
    DB 0x00
PROGLOAD_FILENAME:
    DB "PROGLOADBIN"
MESSAGE_FAILED_TO_LOAD_ROOT_ENTRY:
    DB "Failed to load root entry list!"
    DB 0x0D, 0x0A
    DB 0x00
; 扩展引导器占两个扇区
TIMES 1024 - ($ - $$) db 0