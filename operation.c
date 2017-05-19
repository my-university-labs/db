/* author: dongchangzhang */
/* time: 2017年05月18日 星期四 16时49分11秒 */

#include "operation.h"
#include "extmem.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#define MAX 100000
#define BASE 256

static short mark[MAX];

static unsigned char* format_int_to_byte(u_int source, unsigned char* des)
{
    int i;
    u_int tmp = source;
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

static u_int format_byte_to_int(unsigned char* source)
{
    int i;
    u_int result = 0;
    for (i = 0; i < 4; ++i) {
        result = result * BASE + source[i];
    }
    return result;
}
static u_int get_next_addr()
{
    int num;
    do {
        num = rand() % (MAX);
    } while (mark[num]);
    mark[num] = 1;
    return num;
}
static u_int get_a_data(u_int begin, u_int end)
{
    return rand() % (end - begin) + begin;
}
void init()
{
    srand((unsigned)time(NULL));
    memset(mark, 0, MAX);
}

static int save_start_addr_into_file(char* rel, char* col1, char* col2, u_int start_addr)
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

int get_start_addr_from_file(char* rel, char* col1, char* col2, int r[])
{
    int i;
    u_int addr;
    char buf[255];
    FILE* fp;
    if ((fp = fopen(rel, "r")) == NULL) {
        printf("Can not save the start_addr of relation %s!\n", rel);
        return -1;
    }
    for (i = 0; i < 3; ++i)
        r[i] = 0;
    memset(&buf, 0, sizeof(buf));
    fscanf(fp, "%s %d", buf, &addr);

    r[0] = addr;

    memset(&buf, 0, sizeof(buf));
    fscanf(fp, "%s %d", buf, &addr);
    if (strcmp(buf, col1) == 0) {
        r[1] = addr;
    } else if (strcmp(buf, col2) == 0) {
        r[2] = addr;
    }
    memset(&buf, 0, sizeof(buf));
    fscanf(fp, "%s %d", buf, &addr);
    if (strcmp(buf, col1) == 0) {
        r[1] = addr;
    } else if (strcmp(buf, col2) == 0) {
        r[2] = addr;
    }
    fclose(fp);
    return 0;
}
static u_int do_create_data(u_int start_addr, u_int begin1, u_int end1, u_int begin2, u_int end2, int flag)
{
    int sum = 0;
    int m, n;
    int i, j, k;
    Buffer buf;
    unsigned char tmp[4];
    u_int now_addr, next_addr;
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
            m = get_a_data(begin1, end1);
            format_int_to_byte(m, tmp);
            memcpy(blk + k * 8, tmp, 4);
            n = get_a_data(begin2, end2);
            printf("m: %d, n: %d\n", m, n);
            format_int_to_byte(n, tmp);
            memcpy(blk + k * 8 + 4, tmp, 4);
        }

        /* save addr into the last tuple */
        format_int_to_byte(0, tmp);
        memcpy(blk + k * 8, tmp, 4);
        if (!flag && j == buf.numAllBlk - 1) {
            next_addr = 0;
        } else {
            next_addr = get_next_addr();
        }
        printf("blk index is %d, save into %d; next addr is :%d\n", ++sum, now_addr, next_addr);
        format_int_to_byte(next_addr, tmp);
        memcpy(blk + k * 8 + 4, tmp, 4);

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

u_int create_data(char* rel, char* col1, u_int begin1, u_int end1, char* col2, u_int begin2, u_int end2, u_int times)
{
    printf("------ create date %d blocks for %s ----------------\n", times, rel);
    printf("----------- tuple1 %s: %d ~ %d --------------------\n", col1, begin1, end1);
    printf("----------- tuple2 %s: %d ~ %d --------------------\n", col2, begin2, end2);
    printf("---------------------------------------------------\n");

    int i;
    int sum = 0;
    u_int start_addr = get_next_addr();
    u_int next_addr = start_addr;
    save_start_addr_into_file(rel, col1, col2, start_addr);
    for (i = 0; i < times; ++i) {
        printf("Buffer %d:\n", ++sum);
        next_addr = do_create_data(next_addr, begin1, end1, begin2, end2, times - i - 1);
    }
    return start_addr;
}

u_int read_data(u_int addr, int tuple)
{
    int i;
    u_int num;
    Buffer buf;
    unsigned char tmp[4];
    unsigned char* blk;

    /* init buffer */
    if (!initBuffer(BUFFER_SIZE, BLOCK_SIZE, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    if ((blk = readBlockFromDisk(addr, &buf)) == NULL) {
        perror("Reading Block Failed!\n");
        exit(0);
    }
    for (i = 0; i < 7; ++i) {
        memset(tmp, 0, 4);
        memcpy(tmp, blk + 8 * i + tuple * 4, 4);
        num = format_byte_to_int(tmp);
        printf("%d\n", num);
    }
    memset(tmp, 0, 4);
    memcpy(tmp, blk + 8 * 7 + 4, 4);
    num = format_byte_to_int(tmp);
    printf("addr is %d\n", num);
    return 0;
}
