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
    int i;
    Buffer buf;
    unsigned char *blk, *blk_saver;
    int addr[2], offset, times, data, index_saver = 0;
    /* when find -> save it into there */
    int next_addr, start_addr, save_to;
    start_addr = get_next_addr() + MAX;
    save_to = start_addr;
    /* get start addr of relation */
    get_start_addr_from_file(rel, col, addr);
    offset = addr[1];
    next_addr = addr[0];

    init_buf(&buf);
    blk_saver = getNewBlockInBuffer(&buf);
    /* read blk from disk and save it into buffer */
    while (next_addr) {
        read_blk(next_addr, &buf, &blk);
        /* the number of tuple in this blk */
        times = convert(blk + 8 * 7);
        for (i = 0; i < times; ++i) {
            data = convert(blk + 8 * i + offset * 4);
            if (compare(data, value, op)) {
                save_blk(&buf, &blk_saver, blk + i * 8, &index_saver, &save_to);
            }
        }
        next_addr = convert(blk + 8 * 7 + 4);
        /* free blk */
        freeBlockInBuffer(blk, &buf);
    }
    save_last_blk(&buf, &blk_saver, index_saver, &save_to);

    freeBuffer(&buf);
    return start_addr;
}
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
    int times, index_saver = 0, data, in = 0, end = 0;
    int start_addr, save_to, tmp_addr;
    start_addr = get_next_addr() + TMPBASE + MAX;
    save_to = start_addr;

    unsigned char *blk, *blk_saver = getNewBlockInBuffer(buf);
    while (addr && !end) {
        read_blk(addr, buf, &blk);
        times = convert(blk + 8 * 7);
        tmp_addr = convert(blk + 8 * 7 + 4);
        for (int i = 0; i < times; ++i) {
            data = convert(blk + 8 * i);
            if (compare(data, value, op)) {
                in = 1;
                save_blk(buf, &blk_saver, blk + i * 8, &index_saver, &save_to);

            } else if (in) {
                end = 1;
                break;
            }
        }
        freeBlockInBuffer(blk, buf);
        addr = tmp_addr;
    }
    save_last_blk(buf, &blk_saver, index_saver, &save_to);
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
                save_blk(&buf, &blk_saver, blk + i * 8, &index_saver, &save_to);
            }
        }
        freeBlockInBuffer(blk, &buf);
        addr = tmp_addr;
    }
    save_last_blk(&buf, &blk_saver, index_saver, &save_to);
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
/* 交集 */
int intersect(int addr1, int addr2)
{
    Buffer buf;
    init_buf(&buf);
    unsigned char *blk_a = NULL, *blk_b = NULL, *blk_saver = NULL;
    addr1 = n_merge_sort(addr1, 0);
    addr2 = n_merge_sort(addr2, 0);
    int index_saver = 0;
    blk_saver = getNewBlockInBuffer(&buf);

    unsigned char last_insert[8];
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
            try_to_save_for_set(&buf, &blk_saver, blk_a, offset1, &index_saver, &save_to, last_insert);
            ++offset1;
            ++offset2;
        }
    }
    save_last_blk(&buf, &blk_saver, index_saver, &save_to);
    return start_addr;
}

/* 并集 */
int join(int addr1, int addr2)
{
    Buffer buf;
    init_buf(&buf);
    unsigned char *blk_a = NULL, *blk_b = NULL, *blk_saver = NULL;
    addr1 = n_merge_sort(addr1, 0);
    addr2 = n_merge_sort(addr2, 0);

    int index_saver = 0;
    blk_saver = getNewBlockInBuffer(&buf);

    unsigned char last_insert[8];
    int status1, status2, action, times1 = 0, times2 = 0, offset1 = 0, offset2 = 0;

    int start_addr = TMPBASE + get_next_addr() + MAX;
    int save_to = start_addr;

    memset(last_insert, 1, 8);
    blk_saver = getNewBlockInBuffer(&buf);
    while (1) {
        status1 = check_blk(&buf, &blk_a, &times1, &offset1, &addr1);
        status2 = check_blk(&buf, &blk_b, &times2, &offset2, &addr2);
        action = cmp_tuple(blk_a + offset1 * 8, blk_b + offset2 * 8, 0);
        if ((action == -1 || status2 == -1) && status1 != -1) {
            // a < b, save a
            try_to_save_for_set(&buf, &blk_saver, blk_a, offset1, &index_saver, &save_to, last_insert);
            ++offset1;
        } else if ((action == 1 || status1 == -1) && status2 != -1) {
            // a > b, save b
            try_to_save_for_set(&buf, &blk_saver, blk_b, offset2, &index_saver, &save_to, last_insert);
            ++offset2;
        } else if (action == 0 && status1 != -1 && status2 != -1) {
            try_to_save_for_set(&buf, &blk_saver, blk_a, offset1, &index_saver, &save_to, last_insert);
            ++offset1;
            ++offset2;
        } else {
            break;
        }
    }
    save_last_blk(&buf, &blk_saver, index_saver, &save_to);
    return start_addr;
}
/**
 *  差集
 * set(addr1) - set(addr2)
 * @return start_addr of result
 */
int except(int addr1, int addr2)
{
    Buffer buf;
    init_buf(&buf);
    unsigned char *blk_a = NULL, *blk_b = NULL, *blk_saver = NULL;
    addr1 = n_merge_sort(addr1, 0);
    addr2 = n_merge_sort(addr2, 0);

    int index_saver = 0;
    blk_saver = getNewBlockInBuffer(&buf);

    unsigned char last_insert[8];
    int status1, status2, action, times1 = 0, times2 = 0, offset1 = 0, offset2 = 0;

    int start_addr = TMPBASE + get_next_addr() + MAX;
    int save_to = start_addr;

    memset(last_insert, 1, 8);
    blk_saver = getNewBlockInBuffer(&buf);
    while (addr1 != 0) {
        status1 = check_blk(&buf, &blk_a, &times1, &offset1, &addr1);
        status2 = check_blk(&buf, &blk_b, &times2, &offset2, &addr2);
        action = cmp_tuple(blk_a + offset1 * 8, blk_b + offset2 * 8, 0);
        if ((action == -1 && status1 != -1) || status2 == -1) {
            // a < b && still can get next value from addr1, save a
            // or can not get data from addr2, save a
            try_to_save_for_set(&buf, &blk_saver, blk_a, offset1, &index_saver, &save_to, last_insert);
            ++offset1;
        } else if (action == 1 && status2 != -1) {
            // a > b, move b
            ++offset2;
        } else if (action == 0 && status1 != -1 && status2 != -1) {
            // a == b, pass and move forward
            ++offset1;
            ++offset2;
        } else {
            printf("Else - >\n");
        }
    }
    save_last_blk(&buf, &blk_saver, index_saver, &save_to);
    return start_addr;
}
int nested_loop_join(int addr1, int addr2, int which1, int which2)
{
    Buffer buf;
    int index_saver = 0;
    unsigned char *blk_a, *blk_b, *blk_saver;

    int start_addr, save_to;
    int next1, next2, times1, times2, offset1, offset2;

    start_addr = TMPBASE + get_next_addr();
    save_to = start_addr;

    init_buf(&buf);
    blk_saver = getNewBlockInBuffer(&buf);
    for (next1 = addr1; next1 != 0;) {
        read_blk(next1, &buf, &blk_a);
        times1 = convert(blk_a + 8 * 7);
        next1 = convert(blk_a + 8 * 7 + 4);
        for (next2 = addr2; next2 != 0;) {
            read_blk(next2, &buf, &blk_b);
            times2 = convert(blk_b + 8 * 7);
            next2 = convert(blk_b + 8 * 7 + 4);
            for (offset1 = 0; offset1 < times1; ++offset1) {
                for (offset2 = 0; offset2 < times2; ++offset2) {
                    if (convert(blk_a + offset1 * 8 + which1 * 4) == convert(blk_b + offset2 * 8 + which2 * 4)) {
                        save_blk(&buf, &blk_saver, blk_a + offset1 * 8, &index_saver, &save_to);
                        memcpy(blk_b + offset2 * 8, blk_b + offset2 * 8 + 4, 4);
                        memset(blk_b + offset2 * 8 + 4, 0, 4);
                        save_blk(&buf, &blk_saver, blk_b + offset2 * 8, &index_saver, &save_to);
                    }
                }
            }
            freeBlockInBuffer(blk_b, &buf);
        }
        freeBlockInBuffer(blk_a, &buf);
    }
    save_last_blk(&buf, &blk_saver, index_saver, &save_to);
    read_data(start_addr);
    return start_addr;
}