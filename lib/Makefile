include ../make_def.txt

all: kprintf.o string.o ctype.o

kprintf.o: kprintf.cpp
	$(CXX_COMPILE) kprintf.cpp -o kprintf.o
string.o: string.cpp
	$(CXX_COMPILE) string.cpp -o string.o
ctype.o: ctype.cpp
	$(CXX_COMPILE) ctype.cpp -o ctype.o

clean:
	$(DEL) *.o