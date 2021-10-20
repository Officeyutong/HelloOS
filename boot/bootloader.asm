; DiskGenius新建的FAT32分区，FAT表大小8M，根目录区长度8K，簇大小8192字节

[BITS 16]
    ;C0-H0的下一个柱面是C0-H1

    ORG 0x7c00
    
    JMP main
    ; FAT32 文件系统记录
    DB 0x90; FAT Magic Number
    DB "HELLO-OS" ; 8byte, OEM Name
    DW 512; 扇区大小
cluster_size:
    DB 16  ; Cluster大小(扇区)
fat_start:
    DW 4 ; FAT起始位置
    DB 2 ; FAT表个数
root_entries:
    DW 256 ; 根目录项数
    DW 32768 ; 总扇区个数
    DB 0xF8 ; 磁盘类型
    DW 0; 单个FAT表的长度(FAT12/FAT16)
    DW 0x3F; 每个磁道的扇区个数
    DW 0xFF; 磁头个数
    DD 0; 当前分区的起始LBA
    DD 32768; 总扇区个数，四个字节
    ; 以下部分是FAT12扩展的
    ; DB 0x00; 中断13的驱动器号
    ; DB 0x00; 未使用
    ; DB 0x29 ; 扩展引导标记
    ; DD 0xCAFEBABE; 卷系列号
    ; DB "HELLOOS_FAT" ;卷标，11byte
    ; DB "FAT12", 0, 0, 0; 分区文件系统类型名,8 byte
    ; FAT32扩展记录
fat_length:
    DD 8192 ; 每个FAT表的扇区数
    DW 0; 标志位，留空
    DW 0; FAT版本
root_dir_start:
    DD 2; 根目录表的簇号！！！
    DW 3; FSInfo扇区
    DW 0; 备份的BootSector，0表示无备份
    TIMES 12 DB 0x00; 保留的12个字节
    DB 0x80; 中断13的驱动器号
    DB 0x00; Windows保留
    DB 0x28; 签名
    DD 0xCAFEBABE; 卷序列号
    DB "HELLOOS_FAT" ;卷标，11byte
    DB "FAT32   "; 分区文件系统类型名,8 byte

; 0x7E00 ~ 0x7FFFF - 实模式可用内存
; 0x500 ~ 0x7BFF - 实模式可用内存

; 实模式栈放在 0x8000
; 扩展引导器放在 0x8100 ~ 0x84FF (1024byte)
; FAT表遍历缓存放在 0x8500 ~ 0x86FF (512Byte)

; 根目录表放在 0xA600~0xC5FF(512*16 byte)
; progload放在 0xC100 ~ 0xE0FF ; 最多8K

main:
    MOV SP, 0x8000 ; 栈地址
    MOV AX, 0
    MOV SS, AX

    ; MOV SI, LOADING_STRING
    ; CALL display_string

    ; 
    ; FAT_STORE EQU 0x8200
    ; FAT_TABLE_SIZE EQU 18
    ; ROOT_DIR_BLOCK_SIZE EQU 14
    CYLS_STORE EQU 0x0ff0 ; 读入扇区数
    ROOT_DIR_STORE EQU 0xA600
    PROGLOADER_STORE EQU 0xC100 ; ProgLoader地址
    EXT_BOOTLOADER_STORE EQU 0x8100
    EXT_BOOTLOADER_LENGTH EQU 2


    FUNC_DISPLAY_STRING EQU 0x500
    FUNC_READ_CLUSTER EQU 0x500 + 2
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

    MOV     WORD [FUNC_DISPLAY_STRING], display_string

    MOV     DWORD EAX, [fat_length]
    MOV     DWORD [VAR_FAT_SIZE], EAX
    
    XOR     EAX, EAX
    ; FAT起始扇区
    MOV     WORD AX, [fat_start]
    MOV     WORD [VAR_FAT_START], AX

    ; 根目录区起始扇区
    MOV     DWORD EBX, [VAR_FAT_SIZE]
    SHL     EBX, 1
    XOR     EAX, EAX
    MOV     WORD AX, [VAR_FAT_START]
    ADD     EAX, EBX
    MOV     DWORD [VAR_ROOT_ENTRY_START], EAX
    ; 根目录区长度(扇区)
    MOV     DWORD EAX, [root_entries]
    SHR     EAX, 4
    MOV     WORD [VAR_ROOT_ENTRY_SIZE], AX
    ; 簇大小(扇区)
    XOR     EAX, EAX
    MOV     BYTE AL, [cluster_size]
    MOV     BYTE [VAR_CLUSTER_SIZE_IN_SECTOR], AL
    ; 簇大小(字节)
    SHL     EAX, 9
    MOV     DWORD [VAR_CLUSTER_SIZE_IN_BYTE], EAX
    ; 数据扇区起始
    MOV     DWORD EAX, [VAR_ROOT_ENTRY_START]
    XOR     EBX, EBX
    MOV     WORD BX, [VAR_ROOT_ENTRY_SIZE]
    ADD     EAX, EBX
    MOV     DWORD [VAR_DATA_START_SECTOR], EAX
    ; 根目录区项数
    XOR     EAX, EAX
    MOV     WORD AX, [root_entries]
    MOV     DWORD [VAR_ROOT_DIR_ENTRIES], EAX ; 0x7CC3
    ; MOV WORD [VAR_MESSAGE_PREFIX], MESSAGE_PREFIX
    ; 检查是否支持LBA

    MOV     AH, 0x41
    MOV     BX, 0x55AA
    MOV     DL, 0x80
    INT     0x13
    PUSHFD
    MOV     EAX, [ESP]
    AND     EAX, 1
    CMP     EAX, 0 ; CF是1，不支持
    JNE     unsupported

    JMP     boot_continue


    ALIGNB 4, DB 0x00
LBA_READ_STRUCT: ; 0x7c7c
    DB      16
    DB      0
BLOCK_COUNT:
    DW      EXT_BOOTLOADER_LENGTH ; 读取的块个数
DEST_MEMORY_ADDR:
    DW      EXT_BOOTLOADER_STORE ; 段内偏移
    DW      0 ; 段地址
LBA:
    DD      1 ; 逻辑地址的低32位
    DD      0 ; 逻辑地址的高16位

    ; TOTAL_SHOULD_READ EQU FAT_TABLE_SIZE + ROOT_DIR_BLOCK_SIZE
    ; MOV WORD [BLOCK_COUNT], TOTAL_SHOULD_READ
    ; MOV WORD [DEST_MEMORY_ADDR], FAT_STORE
    ; MOV DWORD [LBA], 1 ; 引导扇区是0
boot_continue:
    MOV     AX, 0
    MOV     DS, AX
    MOV     SI, LBA_READ_STRUCT

    MOV     AH, 0x42
    MOV     DL, 0x80
    INT     0x13
    
    CMP     AH, 0 ; 0x7ca2
    JNE     read_error

    JMP     EXT_BOOTLOADER_STORE

    ; 没那么多空间装判断代码了
    ; MOV BX, [BLOCK_COUNT]
    ; CMP BX, TOTAL_SHOULD_READ
    ; JNE not_enough_sectors

    ; MOV BYTE [CYLS_STORE], FAT_TABLE_SIZE

    ; SUB ESP, 4
    ; MOV SI, PROGLOADER_FILENAME
    ; CALL find_file
    ; MOV AX, DI
    ; CMP AX, 0xFFFF
    ; JNE found_the_file
    ;     MOV SI, FILE_NOT_FOUND
    ;     CALL display_string
    ;     JMP $
    ; found_the_file:

    ; MOV SI, DI
    
    ; MOV AX, PROGLOADER_STORE
    ; MOV DI, AX
    
    ; MOV AX, 0
    ; MOV ES, AX
    ; CALL read_cluster 
    ; JMP PROGLOADER_STORE ; 0x7ccf

unsupported:
    MOV     SI, UNSUPPORTED
    CALL    display_string
    JMP     $

read_error:
    MOV     SI, ERROR_STRING
    CALL    display_string
    JMP     $

; not_enough_sectors:
;     MOV SI, NOT_ENOUGH_SECTOR_STRING
;     CALL display_string
;     JMP $

; clear_screen:
;     MOV AX, 0x0600
;     MOV BX, 0x0700 
;     MOV CX, 0x0000;左上角
;     MOV DX, 0x184f
;     INT 0x10
;     RET

display_string:
    ; PUSH SI
    ; MOV SI, MESSAGE_PREFIX
    ; CALL display_string_raw
    ; POP SI
    ; CALL display_string_raw
    ; RET
display_string_raw: ;显示字符串，SI里存地址，zero-terminated
    MOV     AL,[SI]; 读一个字符
    CMP     AL, 0
    JE      end
    MOV     AH, 0x0e
    MOV     AL,[SI]
    ; MOV BX,15
    INT     0x10
    ADD     SI, 1
    JMP     display_string_raw
end:
    RET


;字符串常量
; MESSAGE_PREFIX:
    ; DB "BOOTLOADER:"
    ; DB 0x00
UNSUPPORTED:
    DB "This FXXKING shit DOES NOT support LBA!"
    DB 0x0D,0x0A
    DB 0x00
ERROR_STRING:
    db "Failed to read disk"
    DB 0x0D,0x0A
    db 0x00
; LOADING_STRING:
;     db "Reading disk..."
;     DB 0x0D,0x0A
;     db 0x00
; OK_STRING:
;     db "Read successfully!"
;     DB 0x0D,0x0A
;     db 0x00
; NOT_ENOUGH_SECTOR_STRING:
;     db "Not enough sectors!"
;     DB 0x0D,0x0A
;     DB 0x00

    times 510 - ($ - $$) db 0

    db    0x55, 0xaa