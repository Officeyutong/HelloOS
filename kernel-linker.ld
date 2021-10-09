
ASSERT(DEFINED(kernel_main), "You should provide the function 'kernal_main'")
SECTIONS 
{
    /*
        前四个字节是主函数偏移
        然后是代码
    */
    . = 0x280000; /* 内核加载的内存位置 */
    kernel_text :
    {
        LONG(kernel_main - 0x280000)
        *(.text)
    }
    /*
        跟在代码后面，对齐8字节

        DATA,
        然后是RODATA
    */
    kernel_data ALIGN(8)  : {
        *(.data)
        *(.rodata)
    }
    ASSERT(. - 0x280000 <= 0x73a00, "The size of kernel_text + kernel_data can not exceed 0x73a00 (from the limit of real mode)")

    kernel_bss ALIGN(8) : {
        *(.bss)
        *(COMMON)
    }
    ASSERT(. - 0x280000 <= 0x100000, "The size of kernel (including bss) can not exceed 1M")
}