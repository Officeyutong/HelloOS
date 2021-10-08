[BITS 16]
    CYLINDER_READ_COUNT EQU 10; 读入总柱面数
    CYLS_STORE EQU 0x0ff0
    ;C0-H0的下一个柱面是C0-H1

    ; ORG 0x7c00
    
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
    DD 2880; 总扇区个数
    DB 0x00,0x00,0x29 ; Magic number
    DD 0xCAFEBABE; 卷系列号
    DB "HELLOOS_FAT" ;分区名，11byte
    DB "FAT12",0,0,0; 分区文件系统类型名,8 byte
    RESB 18; 留空字节

main:
    MOV SP, 0x7F00 ; 栈地址

    MOV SI, LOADING_STRING
    CALL display_string

    MOV AX, 0x0003
    INT 0x10

    MOV AX, 0x0820
    MOV ES, AX; 输入缓冲区地址(ES:BX)


    MOV AX, 0x0201; AH=0x02(读盘),AL=0x01(一个扇区)
    XOR BX, BX; 寻址，清零
    MOV CX, 0x0002; 柱面0，初始扇区2
    MOV DX, 0x0080; DH=磁头0，DL=设备0

readloop:
    MOV SI, 0; 失败次数
retry:
    MOV AX, 0x0201
    XOR BX, BX
    MOV DL, 0x00
    INT 0x13; 读盘
    JNC next
    ADD SI, 1;失败时执行
    CMP SI, 5
    JGE error; 错误过多，失败
    MOV AH, 0x00
    MOV DL, 0x00
    INT 0x13; 重置
    JMP retry
next:
    ;读下一个扇区
    MOV AX, ES
    ADD AX, 0x0020; 缓冲区地址为ES:BX，即ES*16+BX，故这样做相当于地址加了0x200=512字节
    MOV ES, AX
    ADD CL, 1; 下一个扇区
    CMP CL, 18; 一共读18个扇区
    JLE readloop
    ; 下面考虑读第二个柱面了
    MOV CL, 1;第一扇区
    ADD DH, 1;反面的磁头
    CMP DH, 2
    JB readloop; 读到了反面，回去接着读
    ;滚回正面了
    MOV DH, 0
    ADD CH, 1; 下一个柱面
    CMP CH, CYLINDER_READ_COUNT
    JB readloop

    ; MOV SI, OK_STRING
    ; JMP display_string_final
    MOV BYTE [CYLS_STORE], CYLINDER_READ_COUNT
    ; MOV SI, OK_STRING
    ; JMP display_string_final

    JMP 0x8200

error:
    MOV SI, ERROR_STRING
    CALL display_string

display_string: ;显示字符串，SI里存地址，zero-terminated
    ; 先清空屏幕
    MOV AX, 0x0600
    MOV BX, 0x0700 
    MOV CX, 0x0000;左上角
    MOV DX, 0x184f
    INT 0x10
put_loop:
    MOV AL,[SI]; 读一个字符
    CMP AL, 0
    JE end
    MOV AH, 0x0e
    MOV AL,[SI]
    ; MOV BX,15
    INT 0x10
    ADD SI, 1
    JMP put_loop
end:
    RET



;字符串常量
ERROR_STRING:
    db "BOOTLOADER: Failed to read disk"
    db 0x00
LOADING_STRING:
    db "BOOTLOADER: Readling disk..."
    db 0x00
OK_STRING:
    db "Read successfully"
    db 0x00

    times 510 - ($ - $$) db 0

    db    0x55, 0xaa