include ../make_def.txt

all: kprintf.o string.o ctype.o display.o kutil.o kutil-asm.o harddisk.o paging.o idt.o gdt.o interrupt-asm.o interrupt.o keyboard_mouse.o cmos_rtc.o serial.o gdt-cpp.o idt-cpp.o

kprintf.o: kprintf.cpp
	$(CXX_COMPILE) kprintf.cpp -o kprintf.o
string.o: string.cpp
	$(CXX_COMPILE) string.cpp -o string.o
ctype.o: ctype.cpp
	$(CXX_COMPILE) ctype.cpp -o ctype.o
display.o: display.cpp
	$(CXX_COMPILE) display.cpp -o display.o
kutil.o: kutil.cpp
	$(CXX_COMPILE) kutil.cpp -o kutil.o
harddisk.o: harddisk.cpp
	$(CXX_COMPILE) harddisk.cpp -o harddisk.o
paging.o: paging.cpp
	$(CXX_COMPILE) paging.cpp -o paging.o
interrupt.o: interrupt.cpp
	$(CXX_COMPILE) interrupt.cpp -o interrupt.o
keyboard_mouse.o: keyboard_mouse.cpp
	$(CXX_COMPILE) keyboard_mouse.cpp -o keyboard_mouse.o
cmos_rtc.o: cmos_rtc.cpp
	$(CXX_COMPILE) cmos_rtc.cpp -o cmos_rtc.o
serial.o: serial.cpp
	$(CXX_COMPILE) serial.cpp -o serial.o
gdt-cpp.o: gdt.cpp
	$(CXX_COMPILE) gdt.cpp -o gdt-cpp.o
idt-cpp.o: idt.cpp
	$(CXX_COMPILE) idt.cpp -o idt-cpp.o

kutil-asm.o: kutil-asm.asm
	nasm kutil-asm.asm -f elf32 -o kutil-asm.o
gdt.o: gdt.asm
	nasm gdt.asm -f elf32 -o gdt.o
idt.o: idt.asm
	nasm idt.asm -f elf32 -o idt.o
interrupt-asm.o: interrupt-asm.asm
	nasm interrupt-asm.asm -f elf32 -o interrupt-asm.o

clean:
	$(DEL) *.o