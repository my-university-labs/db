/*
 * test.c
 * Zhaonian Zou
 * Harbin Institute of Technology
 * Jun 22, 2011
 */

#include "extmem.h"
#include "operation.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// block size of R
#define R_B 16
// block size of S
#define S_B 32

// size of tuple
#define TUPLE_SIZE 8
// the numbser of tuple of per block
#define TUPLE_PER_BLOCK 7

int main(int argc, char** argv)
{
    int addr[3];
    init();
    //
    char* rrel = "R";
    char* rcol1 = "A";
    char* rcol2 = "B";
    create_data(rrel, rcol1, 1, 40, rcol2, 1, 1000, 2);
    // S
    char* srel = "S";
    char* scol1 = "C";
    char* scol2 = "D";
    create_data(srel, scol1, 20, 60, scol2, 1, 1000, 4);

    get_start_addr_from_file(rrel, rcol1, rcol2, addr);
    printf("%s blk start_addr is %d\n", rrel, addr[0]);
    read_data(addr[0], 0);
    get_start_addr_from_file(srel, scol1, scol2, addr);
    printf("%s blk start_addr is %d\n", srel, addr[0]);
    read_data(addr[0], 1);

    return 0;
}
