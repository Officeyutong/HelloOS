DEL = rm -rf 

bootloader.bin: bootloader.asm 
	nasm -f elf bootloader.asm -o bootloader.o
	ld -m elf_i386 -N bootloader.o --Ttext 0x7c00 --oformat binary -o bootloader.bin
progloader.bin: progloader.asm
	nasm progloader.asm -f bin -o progloader.bin
	# ld -m elf_i386 -N progloader.o --Ttext 0x8200 --oformat=binary -o progloader.bin
asmfunc.o: asmfunc.asm
	nasm asmfunc.asm -f elf32 -o asmfunc.o
cprog.o: cprog.c
	gcc -m32 -march=i386 -ffreestanding -mpreferred-stack-boundary=2 -fno-pic -fno-pie -c cprog.c -Os -o cprog.o
cprog.bin: cprog.o asmfunc.o
	ld -m elf_i386 -N cprog.o asmfunc.o --Ttext 0x00 --oformat=binary -o cprog.bin
cprog.elf: cprog.o asmfunc.o
	ld -m elf_i386 -N cprog.o asmfunc.o --Ttext 0x00 -o cprog.elf
cprog-dump.txt: cprog.elf
	objdump -d cprog.elf > cprog-dump.txt
cprog_with_start.bin: cprog.elf
	python3 make_wrapped_bin.py cprog.elf cprog_with_start.bin
helloos.img: bootloader.bin progloader.bin cprog_with_start.bin
	dd if=/dev/zero of=helloos.img bs=512 count=2880
	dd if=bootloader.bin ibs=512 of=helloos.img count=1 seek=0 conv=notrunc
	dd if=progloader.bin ibs=512 of=helloos.img count=1 seek=1 conv=notrunc
	dd if=cprog_with_start.bin ibs=512 of=helloos.img count=1 seek=2 conv=notrunc
	
run: helloos.img
	qemu-system-i386.exe -m 512 -fda helloos.img -monitor telnet:127.0.0.1:2001,server,nowait
	# qemu-system-i386.exe -drive file=helloos.img,format=raw,if=floppy,index=0
debug: helloos.img
	# killall -9 qemu-system-i386.exe
	qemu-system-i386.exe -fda helloos.img -S -s -monitor telnet:127.0.0.1:2001,server,nowait
run-gdb:
	# 在Windows下开
	gdb \
	-ex 'target remote localhost:1234' \
	-ex 'layout regs' \
	-ex 'break *0x7c00' \
	# --arg qemu-system-i386
	# -ex 'set architecture i8086' \
	# -ex 'set tdesc filename gdb/target.xml' 
	
nasm-dump: bootloader.asm
	nasm bootloader.asm -f bin -o bootloader-nasm.bin	
	objdump -D -b binary -m i8086 bootloader-nasm.bin > bootloader-nasm-dump.S
ld-dump: bootloader.bin
	objdump -D -b binary -m i8086 bootloader.bin > bootloader-dump.S

clean:
	$(DEL) helloos.img
	$(DEL) empty.img
	$(DEL) helloos.elf
	$(DEL) cprog.S
	$(DEL) cprog.asm
	$(DEL) *.o
	$(DEL) *.bin
	$(DEL) bootloader.elf
	$(DEL) cprog.elf
	$(DEL) out.txt
	$(DEL) *-dump
	$(DEL) a.exe
	$(DEL) dump.txt
	$(DEL) cprog-dump.txt