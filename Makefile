all:prepare main
prepare:
	-mkdir -p build
	-mkdir -p blk

main:build/extmem.o build/main.o build/operation.o build/utils.o
	gcc -o main build/main.o build/extmem.o build/operation.o build/utils.o

build/extmem.o:extmem.c extmem.h
	gcc -c $< -o $@

build/test.o:test.c extmem.h
	gcc -c $< -o $@

build/utils.o:utils.c extmem.h utils.h
	gcc -c $< -o $@
build/operation.o:operation.c operation.h  utils.h
	gcc -c $< -o $@

build/main.o:main.c extmem.h operation.h utils.h
	gcc -c $< -o $@

.PHONY:clean blk
clean:
	-rm -rf build/*.o
	-rm -rf main
	-rm -rf blk/*
