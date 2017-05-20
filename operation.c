/* author: dongchangzhang */
/* time: 2017年05月18日 星期四 16时49分11秒 */

#include "operation.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* init mark array and do srand */
void init()
{
    srand((unsigned)time(NULL));
    clear_mark();
}

static int do_create_data(int start_addr, int begin1, int end1, int begin2, int end2, int flag)
{
    int sum = 0;
    int m, n;
    int i, j, k;
    Buffer buf;
    int now_addr, next_addr;
    now_addr = start_addr;
    unsigned char* blk;
    /* init buffer */
    init_buf(&buf);
    for (j = 0; j < buf.numAllBlk; ++j) {
        /* get block */
        blk = getNewBlockInBuffer(&buf);

        /* create date for block */
        for (k = 0; k < 7; ++k) {
            save(blk + k * 8, get_a_data(begin1, end1));
            save(blk + k * 8 + 4, get_a_data(begin2, end2));
        }

        /* save addr into the last tuple */
        save(blk + k * 8, 0);

        if (!flag && j == buf.numAllBlk - 1) {
            next_addr = 0;
        } else {
            next_addr = get_next_addr();
        }
        save(blk + k * 8, 7);

        printf("blk index is %d, save into %d; next addr is :%d\n", ++sum, now_addr, next_addr);
        save(blk + k * 8 + 4, next_addr);

        /* Write the block to the hard disk */

        write_blk(blk, now_addr, &buf);
        now_addr = next_addr;
    }

    freeBuffer(&buf);
    printf("A Buffer Had Been Used!\n");
    return next_addr;
}

int create_data(char* rel, char* col1, int begin1, int end1, char* col2, int begin2, int end2, int times)
{
    printf("---------------------------------------------------\n");
    printf("create date %5d blocks for %s\n", times, rel);
    printf("tuple1 %s: %5d     ~ %5d\n", col1, begin1, end1);
    printf("tuple2 %s: %5d     ~ %5d\n", col2, begin2, end2);
    printf("---------------------------------------------------\n");
    int i;
    int sum = 0;
    int start_addr = get_next_addr();
    int next_addr = start_addr;
    save_start_addr_into_file(rel, col1, col2, start_addr);
    for (i = 0; i < times; ++i) {
        printf("Buffer %d:\n", ++sum);
        next_addr = do_create_data(next_addr, begin1, end1, begin2, end2, times - i - 1);
    }
    return start_addr;
}

int read_data(int addr)
{
    printf("--------------- Read Data ----------------\n");
    int i;
    int num, times;
    Buffer buf;
    unsigned char* blk = NULL;
    /* init buffer */
    init_buf(&buf);
    printf("Read data from blk %d\n", addr);
    while (addr) {
        read_blk(addr, &buf, &blk);
        times = convert(blk + 8 * 7);
        printf("the number of tuple is %d\n", times);

        for (i = 0; i < times; ++i) {
            num = convert(blk + 8 * i);
            printf("%d: %10d, ", i, num);

            num = convert(blk + 8 * i + 4);
            printf("%10d\n", num);
        }

        addr = convert(blk + 8 * 7 + 4);
        printf("next addr of blk is %d\n", addr);
        freeBlockInBuffer(blk, &buf);
    }
    printf("Read End!\n");
    freeBuffer(&buf);
    return 0;
}
int liner_search(char* rel, char* col, int op, int value)
{
    printf("--------------- liner Search ----------------\n");
    int i;
    Buffer buf;
    unsigned char *blk, *blk_save;
    int addr[2], offset, times, data, r_times = 0;
    /* when find -> save it into there */
    int next_addr, result_addr, result_now_addr, result_next_addr;
    result_addr = get_next_addr() + MAX;
    result_now_addr = result_addr;
    /* get start addr of relation */
    get_start_addr_from_file(rel, col, addr);
    offset = addr[1];
    next_addr = addr[0];

    init_buf(&buf);
    blk_save = getNewBlockInBuffer(&buf);
    /* read blk from disk and save it into buffer */
    while (next_addr) {
        printf("Search from blk %d\n", next_addr);
        read_blk(next_addr, &buf, &blk);
        /* the number of tuple in this blk */
        times = convert(blk + 8 * 7);
        for (i = 0; i < times; ++i) {
            data = convert(blk + 8 * i + offset * 4);
            if (compare(data, value, op)) {
                /* data had been found save it */
                printf("-> Find it\n");
                if (r_times >= 7) {
                    /* write result into disk */
                    r_times = 0;
                    save(blk_save + 7 * 8, 7);
                    result_next_addr = get_next_addr() + MAX;
                    save(blk_save + 7 * 8 + 4, result_next_addr);
                    printf("Write result into %d\n", result_now_addr);
                    write_blk(blk_save, result_now_addr, &buf);
                    result_now_addr = result_next_addr;
                    freeBlockInBuffer(blk_save, &buf);
                    blk_save = getNewBlockInBuffer(&buf);
                }
                memcpy(blk_save + r_times * 8, blk + 8 * i, 8);
                ++r_times;
            }
        }
        next_addr = convert(blk + 8 * 7 + 4);
        /* free blk */
        freeBlockInBuffer(blk, &buf);
    }
    save(blk_save + 7 * 8, r_times);
    save(blk_save + 8 * 7 + 4, 0);

    printf("Write result into %d\n", result_now_addr);
    write_blk(blk_save, result_now_addr, &buf);
    printf("Search %s done!\n", rel);
    freeBuffer(&buf);
    return result_addr;
}
