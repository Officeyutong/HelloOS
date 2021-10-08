[BITS 16]
    ;C0-H0的下一个柱面是C0-H1

    ORG 0x7c00
    
    JMP main
    ; FAT12文件系统记录
    DB 0x90; FAT12 Magic Number
    DB "HELLO-OS" ; 8byte, OEM Name
    DW 512; 扇区大小
    DB 1  ; Cluster大小
    DW 1 ; FAT起始位置
    DB 2 ; FAT表个数
    DW 224 ; 根目录项数
    DW 2880 ; 总扇区个数
    DB 0xF0 ; 磁盘类型
    DW 9; 单个FAT表的长度
    DW 18; 每个磁道的扇区个数
    DW 2; 磁头个数
    DD 0; 是否分区，软盘不能分区
    DD 2880; 总扇区个数，四个字节
    ; 以下部分是FAT12扩展的
    DB 0x00; 中断13的驱动器号
    DB 0x00; 未使用
    DB 0x29 ; 扩展引导标记
    DD 0xCAFEBABE; 卷系列号
    DB "HELLOOS_FAT" ;卷标，11byte
    DB "FAT12", 0, 0, 0; 分区文件系统类型名,8 byte
    TIMES 18 DB 0x00; 留空字节

; 0x7E00 ~ 0x7FFFF - 实模式可用内存
; 0x500 ~ 0x7BFF - 实模式可用内存

; 实模式栈放在 0x8000
; FAT表放在 0x8200~0xA5FF (512*18 byte，读两张)
; 根目录表放在 0xA600~0xC0FF(512*14 byte)


main:
    MOV SP, 0x8000 ; 栈地址
    MOV AX, 0
    MOV SS, AX

    ; MOV SI, LOADING_STRING
    ; CALL display_string

    ; 
    FAT_STORE EQU 0x8200
    FAT_TABLE_SIZE EQU 18
    ROOT_DIR_BLOCK_SIZE EQU 14
    CYLS_STORE EQU 0x0ff0 ; 读入扇区数
    ROOT_DIR_STORE EQU 0xA500
    PROGLOADER_STORE EQU 0xC100 ; ProgLoader地址

    FUNC_DISPLAY_STRING EQU 0x500
    FUNC_READ_CLUSTER EQU 0x500 + 2
    FUNC_FIND_FILE EQU 0x500 + 4

    MOV WORD [FUNC_DISPLAY_STRING], display_string
    MOV WORD [FUNC_READ_CLUSTER], read_cluster
    MOV WORD [FUNC_FIND_FILE], find_file
    ; 检查是否支持LBA

    ; MOV AH, 0x41
    ; MOV BX, 0x55AA
    ; MOV DL, 0x80
    ; INT 0x13
    ; PUSHFD
    ; MOV EAX, [ESP]
    ; AND EAX, 1
    ; CMP EAX, 0 ; CF是1，不支持
    ; JNE unsupported


    ALIGNB 4, DB 0x00
LBA_READ_STRUCT: ; 0x7c7c
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

    TOTAL_SHOULD_READ EQU FAT_TABLE_SIZE + ROOT_DIR_BLOCK_SIZE
    MOV WORD [BLOCK_COUNT], TOTAL_SHOULD_READ
    MOV WORD [DEST_MEMORY_ADDR], FAT_STORE
    MOV DWORD [LBA], 1 ; 引导扇区是0
    
    MOV AX, 0
    MOV DS, AX
    MOV SI, LBA_READ_STRUCT

    MOV AH, 0x42
    MOV DL, 0x80
    INT 0x13
    
    CMP AH, 0
    JNE read_error

    ; 没那么多空间装判断代码了
    ; MOV BX, [BLOCK_COUNT]
    ; CMP BX, TOTAL_SHOULD_READ
    ; JNE not_enough_sectors

    MOV BYTE [CYLS_STORE], FAT_TABLE_SIZE

    ; SUB ESP, 4
    MOV SI, PROGLOADER_FILENAME
    CALL find_file
    MOV AX, DI
    CMP AX, 0xFFFF
    JNE found_the_file
        MOV SI, FILE_NOT_FOUND
        CALL display_string
        JMP $
    found_the_file:
    MOV SI, DI
    MOV AX, PROGLOADER_STORE
    MOV DI, AX
    ; MOV WORD AX, [ESP + 2]
    ; MOV WORD [ESP], AX
    ; MOV WORD [ESP + 2], PROGLOADER_STORE
    CALL read_cluster 
    JMP PROGLOADER_STORE
; 从根目录寻找文件
; SI 文件名地址, 2 byte
; OUT DI: 簇号, 2byte (0xFFFF)表示未找到
find_file:
    MOV EAX, ROOT_DIR_STORE ; 根目录表
    ; MOV BP, SP
    PUSH SI
    SUB SP, 2
    MOV WORD [ESP], 0; 循环计数器
scan:
    INC WORD [ESP]
    CMP BYTE [EAX + 11], 0x20 
    JNE scan_continue; 非目录，扫下一个

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
    CMP WORD [ESP], 224
    JE not_found
    ADD DWORD EAX, 32
    JMP scan
not_found:
    MOV WORD DI, 0xFFFF
    JMP scan_exit
found_that:
    ; 记录下目录项簇号
    MOV WORD BX, [EAX + 26]
    MOV WORD DI, BX
scan_exit:
    ADD ESP, 4
    RET

; 读取文件，保存到指定地址
;       SI: 簇号, 2 byte
;       DI: 缓冲地址 , 2byte
read_cluster:
    PUSH DI
    PUSH SI
    XOR ECX, ECX
read_next_cluster:
    MOV WORD BX, [ESP] ; 0x7d25
    ; 当前簇ID在[ESP]
    MOV WORD [BLOCK_COUNT], 1 ; 一个扇区
    MOV AX, [ESP + 2] ; 目标地址
    MOV WORD [DEST_MEMORY_ADDR], AX
    MOV CX, BX
    ; 加上扇区偏移
    ADD CX, 31
    MOV DWORD [LBA], ECX
    MOV AX, 0
    MOV DS, AX
    MOV SI, LBA_READ_STRUCT

    MOV AH, 0x42
    MOV DL, 0x80
    INT 0x13 

    ; 尝试读下一个簇号
    ; 读NMB 半个字节怎么读啊？？
    ; 读完后我把簇号写在[ESP]，缓冲区写在[ESP+2]
    ; c0->0 c1->1
    ; c2->3 c3->4
    ; c4->6
    ; floor(cluster/2)*3+cluster%2

    ; 0xf0 0xf|f 0xff | 0x03 0x4|0 0x00 | 0x05 0x60 0x00
    ; 0xff0 0xfff 0x003 0x004 0x005 0x006
    ; 0x00    0x00    0x00    0x03    0xf0    0xff    0xff    0x0f
    ; x+floor(x/2) ?  
    ; 不管奇偶都从x+floor(x/2)读一个WORD
    ; 偶数与0xfff位与，奇数右移4位
    MOV AX, [ESP] ; 0x7d4f
    MOV BX, AX
    SHR BX, 1
    IMUL BX, 3; BX = 3*floow(AX/3)
    MOV DX, AX
    AND DX, 1
    ; XOR DX, 1 ; DX = AX % 2
    ADD BX, DX
    MOV CX, [BX + FAT_STORE] ; 0x7d61

    ; MOV AX, [ESP]
    ; AND AX, 1

    CMP DX, 0
    JE read_cluster_even
    read_cluster_odd:
        SHR CX, 4
        JMP read_cluster_break
    read_cluster_even:
        AND CX, 0xFFF
    read_cluster_break:
    CMP CX, 0xFFF ; 0x7d71
    JE read_cluster_end
    
    
    MOV [ESP], CX ; 0x7d77
    MOV AX, [ESP + 2]
    ADD AX, 512
    MOV [ESP + 2], AX
    JMP read_next_cluster
read_cluster_end:
    ADD ESP, 4
    RET

; unsupported:
;     MOV SI, UNSUPPORTED
;     CALL display_string
;     JMP $

read_error:
    MOV SI, ERROR_STRING
    CALL display_string
    JMP $

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
    PUSH SI
    MOV SI, BOOTLOADER_PREFIX
    CALL display_string_raw
    POP SI
    CALL display_string_raw
    RET
display_string_raw: ;显示字符串，SI里存地址，zero-terminated
    MOV AL,[SI]; 读一个字符
    CMP AL, 0
    JE end
    MOV AH, 0x0e
    MOV AL,[SI]
    ; MOV BX,15
    INT 0x10
    ADD SI, 1
    JMP display_string_raw
end:
    RET


;字符串常量
BOOTLOADER_PREFIX:
    DB "BOOTLDR: "
    DB 0x00
; UNSUPPORTED:
;     DB "This FXXKING shit DOES NOT support LBA!"
;     DB 0x0D,0x0A
;     DB 0x00
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
FILE_NOT_FOUND:
    DB "PROGLOAD.BIN not found"
    DB 0x0D,0x0A
    DB 0x00
PROGLOADER_FILENAME:
    DB "PROGLOADBIN"

    times 510 - ($ - $$) db 0

    db    0x55, 0xaa