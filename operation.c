/* author: dongchangzhang */
/* time: 2017年05月18日 星期四 16时49分11秒 */

#include "operation.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#define MAX 100000
#define BASE 256

static short mark[MAX];
/* int byte */
static unsigned char* format_int_to_byte(int source, unsigned char* des)
{
    int i;
    int tmp = source;
    memset(des, 0, 4);
    for (i = 0; i < 4; ++i) {
        tmp = source % BASE;
        source /= BASE;
        des[4 - i - 1] = tmp;
        if (source == 0)
            return des;
    }
    perror("Error when format int to byte!\n");
    return NULL;
}
/* byte to int */
static int format_byte_to_int(unsigned char* source)
{
    int i;
    int result = 0;
    for (i = 0; i < 4; ++i) {
        result = result * BASE + source[i];
    }
    return result;
}
static int convert(unsigned char* blk)
{
    unsigned char tmp[3];
    memset(tmp, 0, 4);
    memcpy(tmp, blk, 4);
    return format_byte_to_int(tmp);
}
static void save(unsigned char* blk, int num)
{
    unsigned char tmp[3];
    format_int_to_byte(num, tmp);
    memcpy(blk, tmp, 4);
}
/* create an addr to save blk */
static int get_next_addr()
{
    int num;
    do {
        num = rand() % (MAX);
    } while (mark[num]);
    mark[num] = 1;
    return num;
}

/* create data for tuple */
static int get_a_data(int begin, int end)
{
    int n = rand() % (end + 1 - begin) + begin;
    if (n % 5 == 0 && end <= 40)
        return 40;
    else if (n % 5 == 0 && end <= 60)
        return 60;
    return n;
    // return 40;
}
/* init mark array and do srand */
void init()
{
    srand((unsigned)time(NULL));
    memset(mark, 0, MAX);
}
/* save the addr of the first blk for relation */
static int save_start_addr_into_file(char* rel, char* col1, char* col2, int start_addr)
{
    FILE* fp;
    if ((fp = fopen(rel, "w")) == NULL) {
        printf("Can not save the start_addr of relation %s!\n", rel);
        return -1;
    }
    fprintf(fp, "%s %d %s %d %s %d", rel, start_addr, col1, 0, col2, 1);
    fclose(fp);
    return 0;
}
/* read addr for relation */
int get_start_addr_from_file(char* rel, char* col, int r[])
{
    int i;
    int addr;
    char buf[255];
    FILE* fp;
    if ((fp = fopen(rel, "r")) == NULL) {
        printf("Can not save the start_addr of relation %s!\n", rel);
        return -1;
    }
    for (i = 0; i < 2; ++i)
        r[i] = 0;
    memset(&buf, 0, sizeof(buf));
    fscanf(fp, "%s %d", buf, &addr);

    r[0] = addr;

    memset(&buf, 0, sizeof(buf));
    fscanf(fp, "%s %d", buf, &addr);
    if (strcmp(buf, col) == 0) {
        r[1] = addr;
    } else if (strcmp(buf, col) == 0) {
        r[2] = addr;
    }
    fclose(fp);
    return 0;
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
    if (!initBuffer(BUFFER_SIZE, BLOCK_SIZE, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
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
        if (writeBlockToDisk(blk, now_addr, &buf) != 0) {
            perror("Writing Block Failed!\n");
            return -1;
        }
        now_addr = next_addr;
    }

    freeBuffer(&buf);
    printf("A Buffer Had Been Used!\n");
    return next_addr;
}

static int compare(int value1, int value2, int op)
{
    switch (op) {
    case EQ:
        return value1 == value2;
    case GT:
        return value1 > value2;
    case LT:
        return value1 < value2;
    case GE:
        return value1 >= value2;
    case LE:
        return value1 <= value2;
    default:
        printf("Operation Error!\n");
        return -1;
    }
}
int create_data(char* rel, char* col1, int begin1, int end1, char* col2, int begin2, int end2, int times)
{
    printf("------ create date %d blocks for %s ----------------\n", times, rel);
    printf("----------- tuple1 %s: %d ~ %d --------------------\n", col1, begin1, end1);
    printf("----------- tuple2 %s: %d ~ %d --------------------\n", col2, begin2, end2);
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
    unsigned char* blk;

    /* init buffer */
    if (!initBuffer(BUFFER_SIZE, BLOCK_SIZE, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    printf("Read data from blk %d\n", addr);

    while (addr) {
        if ((blk = readBlockFromDisk(addr, &buf)) == NULL) {
            perror("Reading Block Failed!\n");
            exit(0);
        }
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
    int i, num;
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

    if (!initBuffer(BUFFER_SIZE, BLOCK_SIZE, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    blk_save = getNewBlockInBuffer(&buf);
    /* read blk from disk and save it into buffer */
    while (next_addr) {
        printf("Search from blk %d\n", next_addr);

        if ((blk = readBlockFromDisk(next_addr, &buf)) == NULL) {
            perror("Reading Block Failed!\n");
            exit(0);
        }
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

                    if (writeBlockToDisk(blk_save, result_now_addr, &buf) != 0) {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
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
    if (writeBlockToDisk(blk_save, result_now_addr, &buf) != 0) {
        perror("Writing Block Failed!\n");
        return -1;
    }
    printf("Search %s done!\n", rel);
    freeBuffer(&buf);

    return result_addr;
}
int merge(int addr1, int addr2)
{
    Buffer buf;
    unsigned char* blk;
    /* init buffer */
    if (!initBuffer(BUFFER_SIZE, BLOCK_SIZE, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int next = addr1, last;
    while (next) {
        if (buf.numFreeBlk < 2)
            freeBlockInBuffer(blk, &buf);
        blk = getNewBlockInBuffer(&buf);
        if ((blk = readBlockFromDisk(next, &buf)) == NULL) {
            perror("Reading Block Failed!\n");
            exit(0);
        }
        last = next;
        next = convert(blk + 8 * 7 + 4);
        printf("next addr of blk is %d\n", next);
    }
    dropBlockOnDisk(last);
    save(blk + 7 * 8 + 4, addr2);
    if (writeBlockToDisk(blk, last, &buf) != 0) {
        perror("Writing Block Failed!\n");
        return -1;
    }
    freeBuffer(&buf);
    return addr1;
}