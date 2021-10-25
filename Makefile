include make_def.txt

LINK_FILES = ckernel.o ./lib/ctype.o ./lib/display.o ./lib/kprintf.o ./lib/string.o ./lib/kutil.o ./lib/kutil-asm.o ./lib/harddisk.o ./lib/paging.o ./lib/gdt.o ./lib/idt.o ./lib/interrupt-asm.o ./lib/interrupt.o ./lib/keyboard_mouse.o ./lib/cmos_rtc.o ./lib/serial.o ./lib/gdt-cpp.o ./lib/idt-cpp.o


ascii_font.bin: make_font.py hankaku.txt
	python3 make_font.py hankaku.txt ascii_font.bin


progloader.bin: progloader.asm
	nasm progloader.asm -f bin -o progloader.bin
	# ld -m elf_i386 -N progloader.o --Ttext 0x8200 --oformat=binary -o progloader.bin
progload.bin: progloader.bin
	cp progloader.bin progload.bin

ckernel.o: ckernel.cpp
	$(CXX_COMPILE) ckernel.cpp -o ckernel.o

kernel-lib:
	cd lib && $(MAKE)

boot-program:
	cd boot && $(MAKE)

kernel.bin: ckernel.o kernel-lib kernel-linker.ld
	ld --oformat binary -m elf_i386 -T kernel-linker.ld  \
		$(LINK_FILES) \
		-o kernel.bin \
		-L /usr/lib/gcc/x86_64-linux-gnu/9/32 -l gcc
kernel.elf: ckernel.o kernel-lib kernel-linker.ld
	ld --oformat elf32-i386 -m elf_i386 -T kernel-linker.ld  \
		$(LINK_FILES) \
		-o kernel.elf \
		-L /usr/lib/gcc/x86_64-linux-gnu/9/32 -l gcc
kernel-dump-text.txt: kernel.elf
	objdump --demangle -j kernel_text -d kernel.elf > kernel-dump-text.txt
kernel-dump-data.txt: kernel.elf
	objdump --demangle -j kernel_data -d kernel.elf > kernel-dump-data.txt
kernel-dump-header.txt: kernel.elf
	objdump --demangle -h kernel.elf > kernel-dump-header.txt
kernel-dump: kernel-dump-text.txt kernel-dump-data.txt kernel-dump-header.txt
	# nothing  
kernel-dump.txt: kernel.elf
	objdump -d kernel.elf > kernel.txt

helloos-empty-fat32.img: boot-program
	dd if=/dev/zero of=helloos-empty-fat32.img bs=512 count=32768
	dd if=./boot/bootloader.bin ibs=512 of=helloos-empty-fat32.img count=1 seek=0 conv=notrunc
	dd if=./boot/bootloader-ext.bin ibs=512 of=helloos-empty-fat32.img count=2 seek=1 conv=notrunc
	dd if=./boot/fsinfo.bin ibs=512 of=helloos-empty-fat32.img count=1 seek=3 conv=notrunc
	dd if=./boot/empty_fat32.bin ibs=512 of=helloos-empty-fat32.img count=1 seek=4 conv=notrunc
	dd if=./boot/empty_fat32.bin ibs=512 of=helloos-empty-fat32.img count=1 seek=8196 conv=notrunc
	
	
	
helloos-fat32.img: helloos-empty-fat32.img progload.bin kernel.bin ascii_font.bin
	cp helloos-empty-fat32.img helloos-fat32.img
	mcopy -i helloos-fat32.img progload.bin ::
	mcopy -i helloos-fat32.img kernel.bin ::
	mcopy -i helloos-fat32.img ascii_font.bin ::
	
helloos.vhd: helloos-fat32.img
	qemu-img.exe convert -f raw -O vpc helloos-fat32.img helloos.vhd
helloos.qcow2: helloos-fat32.img
	qemu-img.exe convert -f raw -O qcow2 helloos-fat32.img helloos.qcow2

# helloos.img: bootloader.bin progloader.bin cprog_with_start.bin
# 	dd if=/dev/zero of=helloos.img bs=512 count=2880
# 	dd if=bootloader.bin ibs=512 of=helloos.img count=1 seek=0 conv=notrunc
# 	dd if=progloader.bin ibs=512 of=helloos.img count=1 seek=1 conv=notrunc
# 	dd if=cprog_with_start.bin ibs=512 of=helloos.img count=1 seek=2 conv=notrunc
	
runx: helloos-fat32.img
	qemu-system-i386.exe -m 512 -hda helloos-fat32.img -monitor telnet:127.0.0.1:2001,server,nowait -rtc base=localtime
debugx: helloos-fat32.img
	qemu-system-i386.exe -m 512 -hda helloos-fat32.img -S -gdb tcp::2002 -monitor telnet:127.0.0.1:2001,server,nowait -rtc base=localtime
run-gdb: kernel.elf
	gdb kernel.elf \
	-ex 'target remote 127.0.0.1:2002' \
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
	$(DEL) helloos-empty-fat32.img
	$(DEL) helloos-fat32.img
	$(DEL) kernel-dump-*.txt
	$(DEL) kernel.elf
	$(DEL) ascii_font.bin
	$(DEL) helloos.qcow2
	$(DEL) helloos.vhd
	$(DEL) fuck.sh
	$(DEL) err.txt

	cd lib && $(MAKE) clean
	cd boot && $(MAKE) clean