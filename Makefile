DEL = rm -rf 

bootloader.bin: bootloader.asm 
	nasm -f bin bootloader.asm -o bootloader.bin
	# ld -m elf_i386 -N bootloader.o --Ttext 0x7c00 --oformat binary -o bootloader.bin
progloader.bin: progloader.asm
	nasm progloader.asm -f bin -o progloader.bin
	# ld -m elf_i386 -N progloader.o --Ttext 0x8200 --oformat=binary -o progloader.bin
progload.bin: progloader.bin
	cp progloader.bin progload.bin
asmfunc.o: asmfunc.asm
	nasm asmfunc.asm -f elf32 -o asmfunc.o
ckernel.o: ckernel.c
	gcc -m32 -march=i386 -ffreestanding -mpreferred-stack-boundary=2 -fno-pic -fno-pie -c ckernel.c -o ckernel.o
kernel.bin: ckernel.o asmfunc.o kernel-linker.ld
	ld --no-eh-frame-hdr -m elf_i386 --oformat binary -T kernel-linker.ld ckernel.o asmfunc.o -o kernel.bin
kernel.elf: ckernel.o asmfunc.o kernel-linker.ld
	ld --no-eh-frame-hdr -m elf_i386 --oformat elf32-i386 -T kernel-linker.ld ckernel.o asmfunc.o -o kernel.elf
kernel-dump-text.txt: kernel.elf
	objdump -j kernel_text -d kernel.elf > kernel-dump-text.txt
kernel-dump-data.txt: kernel.elf
	objdump -j kernel_data -d kernel.elf > kernel-dump-data.txt
kernel-dump-header.txt: kernel.elf
	objdump -h kernel.elf > kernel-dump-header.txt
kernel-dump: kernel-dump-text.txt kernel-dump-data.txt kernel-dump-header.txt
	# nothing  
kernel-dump.txt: kernel.elf
	objdump -d kernel.elf > kernel.txt

helloos-empty-fat12.img: bootloader.bin
	dd if=/dev/zero of=helloos-empty-fat12.img bs=512 count=2880
	dd if=bootloader.bin ibs=512 of=helloos-empty-fat12.img count=1 seek=0 conv=notrunc
helloos-fat12.img: helloos-empty-fat12.img progload.bin kernel.bin
	cp helloos-empty-fat12.img helloos-fat12.img
	mcopy -i helloos-fat12.img progload.bin ::
	mcopy -i helloos-fat12.img kernel.bin ::
	
# helloos.img: bootloader.bin progloader.bin cprog_with_start.bin
# 	dd if=/dev/zero of=helloos.img bs=512 count=2880
# 	dd if=bootloader.bin ibs=512 of=helloos.img count=1 seek=0 conv=notrunc
# 	dd if=progloader.bin ibs=512 of=helloos.img count=1 seek=1 conv=notrunc
# 	dd if=cprog_with_start.bin ibs=512 of=helloos.img count=1 seek=2 conv=notrunc
	
runx: helloos-fat12.img
	qemu-system-i386.exe -m 512 -hda helloos-fat12.img -monitor telnet:127.0.0.1:2001,server,nowait
debugx: helloos-fat12.img
	qemu-system-i386.exe -m 512 -hda helloos-fat12.img -S -s -monitor telnet:127.0.0.1:2001,server,nowait
run: helloos.img
	qemu-system-i386.exe -m 512 -hda helloos.img -monitor telnet:127.0.0.1:2001,server,nowait
	# qemu-system-i386.exe -drive file=helloos.img,format=raw,if=floppy,index=0
debug: helloos.img
	qemu-system-i386.exe -hda helloos.img -S -s -monitor telnet:127.0.0.1:2001,server,nowait
run-gdb:
	# 在Windows下开
	gdb \
	-ex 'target remote localhost:1234' \
	-ex 'layout regs' \
	-ex 'break *0x7c00' 
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
	$(DEL) helloos-empty-fat12.img
	$(DEL) helloos-fat12.img
	$(DEL) kernel-dump-*.txt