include ../make_def.txt

all: bootloader.bin bootloader-ext.bin fsinfo.bin empty_fat32.bin


bootloader-ext.bin: bootloader-ext.asm
	nasm -f bin bootloader-ext.asm -o bootloader-ext.bin
fsinfo.bin: fsinfo.asm
	nasm -f bin fsinfo.asm -o fsinfo.bin
bootloader.bin: bootloader.asm 
	nasm -f bin bootloader.asm -o bootloader.bin
empty_fat32.bin: empty_fat32.asm
	nasm -f bin empty_fat32.asm -o empty_fat32.bin

clean: 
	$(DEL) *.o
	$(DEL) *.bin
