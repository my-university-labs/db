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
    int j, k, sum = 0;
    Buffer buf;
    int now_addr, next_addr;
    now_addr = start_addr;
    unsigned char* blk;
    /* init buffer */
    init_buf(&buf);
    for (j = 0; j < 8; ++j) {
        /* get block */
        blk = getNewBlockInBuffer(&buf);

        /* create date for block */
        for (k = 0; k < 7; ++k) {
            save(blk + k * 8, get_a_data(begin1, end1));
            save(blk + k * 8 + 4, get_a_data(begin2, end2));
        }

        /* save addr into the last tuple */
        next_addr = get_next_addr();
        save(blk + 7 * 8, 7);
        if (j == 7 && flag == 0)
            save(blk + 7 * 8 + 4, 0);
        else
            save(blk + 7 * 8 + 4, next_addr);
        /* Write the block to the hard disk */
        write_blk(blk, now_addr, &buf);
        // read_a_data(now_addr);
        now_addr = next_addr;
    }
    freeBuffer(&buf);
    return next_addr;
}

int create_data(char* rel, char* col1, int begin1, int end1, char* col2, int begin2, int end2, int times)
{
    int i;
    int sum = 0;
    int start_addr = get_next_addr();
    int next_addr = start_addr;
    save_start_addr_into_file(rel, col1, col2, start_addr);
    for (i = 0; i < times; ++i) {
        next_addr = do_create_data(next_addr, begin1, end1, begin2, end2, times - i - 1);
    }
    return start_addr;
}

int read_data(int addr)
{
    int i, j = 0;
    int num, times;
    Buffer buf;
    unsigned char* blk = NULL;
    /* init buffer */
    init_buf(&buf);
    while (addr) {
        read_blk(addr, &buf, &blk);
        times = convert(blk + 8 * 7);
        printf("Read data from blk %d\n", addr);
        printf("%d the number of tuple is %d\n", ++j, times);

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
int read_a_data(int addr)
{
    printf("--------------- Read A Data ----------------\n");
    int i;
    int num, times;
    Buffer buf;
    unsigned char* blk = NULL;
    /* init buffer */
    init_buf(&buf);
    printf("Read data from blk %d\n", addr);
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
        read_blk(next_addr, &buf, &blk);
        /* the number of tuple in this blk */
        times = convert(blk + 8 * 7);
        for (i = 0; i < times; ++i) {
            data = convert(blk + 8 * i + offset * 4);
            if (compare(data, value, op)) {
                if (r_times >= 7) {
                    /* write result into disk */
                    r_times = 0;
                    save(blk_save + 7 * 8, 7);
                    result_next_addr = get_next_addr() + MAX;
                    save(blk_save + 7 * 8 + 4, result_next_addr);
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

    write_blk(blk_save, result_now_addr, &buf);
    freeBuffer(&buf);
    return result_addr;
}
/**
 *
*/
static int move_pointer(int addr, int value, Buffer* buf)
{
    unsigned char* blk;
    read_blk(addr, buf, &blk);
    int nums = convert(blk + 7 * 8);
    int start = convert(blk);
    int end = convert(blk + (nums - 1) * 8);
    freeBlockInBuffer(blk, buf);
    if (compare(start, value, GE))
        return -1;
    if (compare(end, value, LT))
        return 1;
    return 0;
}
static int save_data_for_b_search(Buffer* buf, int addr, int op, int value)
{
    int times, timer = 0, data, in = 0, end = 0;
    int start_addr, now_addr, next_addr, tmp_addr;
    start_addr = get_next_addr() + TMPBASE + MAX;
    now_addr = start_addr;

    unsigned char *blk, *blk_save = getNewBlockInBuffer(buf);
    while (addr && !end) {
        read_blk(addr, buf, &blk);
        times = convert(blk + 8 * 7);
        tmp_addr = convert(blk + 8 * 7 + 4);
        for (int i = 0; i < times; ++i) {
            data = convert(blk + 8 * i);
            if (compare(data, value, op)) {
                in = 1;
                if (timer >= 7) {
                    timer = 0;
                    save(blk_save + 8 * 7, 7);
                    next_addr = get_next_addr() + TMPBASE + MAX;
                    save(blk_save + 7 * 8 + 4, next_addr);
                    write_blk(blk_save, now_addr, buf);
                    now_addr = next_addr;
                    blk_save = getNewBlockInBuffer(buf);
                }
                memcpy(blk_save + timer * 8, blk + 8 * i, 8);
                ++timer;
            } else if (in) {
                end = 1;
                break;
            }
        }
        freeBlockInBuffer(blk, buf);
        addr = tmp_addr;
    }
    save(blk_save + 8 * 7, timer);
    save(blk_save + 7 * 8 + 4, 0);
    write_blk(blk_save, now_addr, buf);
    return start_addr;
}
int binary_search(char* rel, char* col, int op, int value)
{
    FILE* fp;
    char filename[64];
    int addr_saver[2];
    int times = 0, tmp;
    int addr_buf[1000];

    Buffer buf;
    init_buf(&buf);

    get_start_addr_from_file(rel, col, addr_saver);
    int addr = n_merge_sort(addr_saver[0], 0);
    sprintf(filename, "blk/index%d.blk", addr);
    fp = fopen(filename, "r");
    while (fscanf(fp, "%d", &tmp) != EOF) {
        addr_buf[times++] = tmp;
    }
    int start = 0, end = times - 1, mid, action;
    while (start <= end) {
        mid = (start + end) / 2;
        action = move_pointer(addr_buf[mid], value, &buf);
        if (action == -1) {
            end = mid - 1;
        } else if (action == 1) {
            start = mid + 1;
        } else {
            break;
        }
    }

    int t = save_data_for_b_search(&buf, addr_buf[mid], EQ, value);
    fclose(fp);
    return t;
}
/* 投影操作 */
int project(char* rel, char* col)
{
    Buffer buf;
    short mark[1000];
    int addr_saver[2];
    init_buf(&buf);
    for (int i = 0; i <= 1000; ++i) {
        mark[i] = 0;
    }
    get_start_addr_from_file(rel, col, addr_saver);
    int addr = addr_saver[0], offset = addr_saver[1];

    int times, index_saver = 0, data;
    int start_addr, save_to, tmp_addr;
    start_addr = get_next_addr() + TMPBASE + MAX;
    save_to = start_addr;

    unsigned char *blk, *blk_saver = getNewBlockInBuffer(&buf);
    while (addr) {
        read_blk(addr, &buf, &blk);
        times = convert(blk + 8 * 7);
        tmp_addr = convert(blk + 8 * 7 + 4);
        for (int i = 0; i < times; ++i) {
            data = convert(blk + 8 * i + offset * 4);
            if (!mark[data]) {
                mark[data] = 1;
                save_info(&buf, &blk_saver, blk + i * 8, &index_saver, 0, &save_to, 0);
            }
        }
        freeBlockInBuffer(blk, &buf);
        addr = tmp_addr;
    }
    save_info(&buf, &blk_saver, NULL, NULL, index_saver, &save_to, 1);
    return start_addr;
}

static int check_blk(Buffer* buf, unsigned char** blk, int* times, int* offset, int* next)
{
    if (*offset < *times) {
        return 0;
    }
    if (*next == 0) {
        return -1;
    }
    if (*blk != NULL) {
        freeBlockInBuffer(*blk, buf);
    }
    read_blk(*next, buf, blk);
    *times = convert(*blk + 7 * 8);
    *next = convert(*blk + 7 * 8 + 4);
    *offset = 0;
    return 0;
}

int combine(int addr1, int addr2)
{
    Buffer buf;
    init_buf(&buf);
    unsigned char *blk_a = NULL, *blk_b = NULL, *blk_saver = NULL;
    addr1 = n_merge_sort(addr1, 0);
    addr2 = n_merge_sort(addr2, 0);
    int index_saver = 0;
    blk_saver = getNewBlockInBuffer(&buf);

    char last_insert[8];
    int status1, status2, action, times1 = 0, times2 = 0, offset1 = 0, offset2 = 0;

    int start_addr = TMPBASE + get_next_addr() + MAX;
    int save_to = start_addr;

    memset(last_insert, 1, 8);
    blk_saver = getNewBlockInBuffer(&buf);
    while (1) {
        status1 = check_blk(&buf, &blk_a, &times1, &offset1, &addr1);
        status2 = check_blk(&buf, &blk_b, &times2, &offset2, &addr2);
        action = cmp_tuple(blk_a + offset1 * 8, blk_b + offset2 * 8, 0);
        if (action == -1) {
            // a < b, move a
            if (status1 == -1)
                break;
            ++offset1;
        } else if (action == 1) {
            // a > b, move b
            if (status2 == -1)
                break;
            ++offset2;
        } else {
            // a = b, check
            if (memcmp(blk_a + offset1 * 8, last_insert, 8) != 0) {
                // save it
                save_info(&buf, &blk_saver, blk_a + offset1 * 8, &index_saver, 0, &save_to, 0);
                memcpy(last_insert, blk_a + offset1 * 8, 8);
            }
            ++offset1;
            ++offset2;
        }
    }
    save_info(&buf, &blk_saver, NULL, NULL, index_saver, &save_to, 1);
    return start_addr;
}