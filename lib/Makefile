include ../make_def.txt

all: kprintf.o string.o ctype.o display.o asmfunc.o kutil.o kutil-asm.o harddisk.o paging.o

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

asmfunc.o: asmfunc.asm
	nasm asmfunc.asm -f elf32 -o asmfunc.o
kutil-asm.o: kutil-asm.asm
	nasm kutil-asm.asm -f elf32 -o kutil-asm.o

clean:
	$(DEL) *.o