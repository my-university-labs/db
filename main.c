#include "extmem.h"
#include "operation.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv)
{
    int addr[2];
    init();
    //
    char* rrel = "R";
    char* rcol1 = "A";
    char* rcol2 = "B";
    int r = create_data(rrel, rcol1, 1, 40, rcol2, 1, 1000, 2);
    // S
    char* srel = "S";
    char* scol1 = "C";
    char* scol2 = "D";
    int s = create_data(srel, scol1, 20, 60, scol2, 1, 1000, 4);

    read_data(r);

    read_data(s);
printf("==================================\n");
    int line_s_r = liner_search(rrel, rcol1, EQ, 40);

    int line_s_s = liner_search(srel, scol1, EQ, 60);

    read_data(line_s_r);

    read_data(line_s_s);

    int addr_merge = merge(line_s_r, line_s_s);

    read_data(addr_merge);

    return 0;
}
