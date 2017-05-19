all:test main

test:extmem.o test.o
	gcc -o test test.o extmem.o

main:extmem.o main.o operation.o
	gcc -o main main.o extmem.o operation.o

extmem.o:extmem.c extmem.h
	gcc -c extmem.c

test.o:test.c extmem.h
	gcc -c test.c

operation.o:operation.c operation.h extmem.h
	gcc -c operation.c

main.o:main.c extmem.h operation.h
	gcc -c main.c

clean:
	-rm -rf *.o
	-rm -rf test
	-rm -rf main

blk:
	-rm -rf *.blk
