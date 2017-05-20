/* author: dongchangzhang */
/* time: Sat 20 May 2017 02:58:46 PM CST */
#include "utils.h"
#include "operation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static short mark[MAX];

static short qoffset = 0;

void clear_mark()
{
    memset(mark, 0, MAX);
}
/* input blk pointer and get the value saved in there */
int convert(unsigned char* blk)
{
    int result = 0;
    memcpy(&result, blk, sizeof(int));
    return result;
}
/* save num into addr -> blk*/
void save(unsigned char* blk, int num)
{
    memcpy(blk, &num, sizeof(int));
}

void init_buf(Buffer* buf)
{
    if (!initBuffer(BUFFER_SIZE, BLOCK_SIZE, buf)) {
        perror("Buffer Initialization Failed!\n");
        exit(1);
    }
}
void write_blk(unsigned char* blk, unsigned int addr, Buffer* buf)
{
    if (writeBlockToDisk(blk, addr, buf) != 0) {
        perror("Writing Block Failed!\n");
        exit(2);
    }
}
void read_blk(unsigned int addr, Buffer* buf, unsigned char** blk)
{
    if ((*blk = readBlockFromDisk(addr, buf)) == NULL) {
        perror("Reading Block Failed!\n");
        exit(3);
    }
}
/* create an addr to save blk */
int get_next_addr()
{
    int num;
    do {
        num = rand() % (MAX);
    } while (mark[num]);
    mark[num] = 1;
    return num;
}

/* create data for tuple */
int get_a_data(int begin, int end)
{
    int n = rand() % (end + 1 - begin) + begin;
    if (n % 15 == 0 && end <= 40)
        return 40;
    else if (n % 15 == 0 && end <= 60)
        return 60;
    return n;
}
/* save the addr of the first blk for relation */
int save_start_addr_into_file(char* rel, char* col1, char* col2, int start_addr)
{
    FILE* fp;
    char filename[40];
    sprintf(filename, "blk/%s", rel);
    if ((fp = fopen(filename, "w")) == NULL) {
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
    FILE* fp;
    char buf[64];
    int addr, addr1, addr2;
    sprintf(buf, "blk/%s", rel);
    if ((fp = fopen(buf, "r")) == NULL) {
        printf("Can not save the start_addr of relation %s!\n", rel);
        return -1;
    }
    memset(&buf, 0, sizeof(buf));
    fscanf(fp, "%s %d %s %d %s %d", &buf[0], &addr, &buf[20], &addr1, &buf[40], &addr2);
    fclose(fp);

    r[0] = addr;
    if (strcmp(&buf[20], col) == 0) {
        r[1] = addr1;
    } else if (strcmp(&buf[40], col) == 0) {
        r[1] = addr2;
    } else {
        perror("Error when find column\n");
        exit(4);
    }
    return 0;
}
/* merge addr1 and addr2 */
int link_addr(int addr1, int addr2)
{
    Buffer buf;
    unsigned char* blk;
    /* init buffer */
    init_buf(&buf);

    int next = addr1, last;
    while (next) {
        if (buf.numFreeBlk < 2)
            freeBlockInBuffer(blk, &buf);
        blk = getNewBlockInBuffer(&buf);
        read_blk(next, &buf, &blk);
        last = next;
        next = convert(blk + 8 * 7 + 4);
    }
    dropBlockOnDisk(last);
    save(blk + 7 * 8 + 4, addr2);
    write_blk(blk, last, &buf);
    freeBuffer(&buf);
    printf("Merge done!\n");
    return addr1;
}
int compare(int value1, int value2, int op)
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
static int cmp_blk_item(const void* a, const void* b)
{
    int tmp1 = 0, tmp2 = 0;
    int tmp3 = 0, tmp4 = 0;
    memcpy(&tmp1, a, sizeof(int));
    memcpy(&tmp2, b, sizeof(int));
    memcpy(&tmp3, (a + 4), sizeof(int));
    memcpy(&tmp4, (b + 4), sizeof(int));
    if (qoffset == 0)
        return tmp1 == tmp2 ? tmp3 > tmp4 : tmp1 > tmp2;
    else
        return tmp3 == tmp4 ? tmp1 > tmp2 : tmp3 > tmp4;
}

void blk_sort(unsigned char* blk, int times, int offset)
{
    if (blk == NULL) {
        perror("Buffer Memory Error\n");
    }
    qoffset = offset;
    qsort(blk, times, 2 * sizeof(int), cmp_blk_item);
}
static void init_tmp(TmpSegments* tmps)
{
    tmps->times = 0;
    for (int i = 0; i < TMP_SIZE; ++i) {
        tmps->flag[i] = FALSE;
    }
}
static int insert_tmp(TmpSegments* tmps, unsigned int addr)
{
    if (tmps->times >= TMP_SIZE) {
        return -1;
    }
    for (int i = 0; i < TMP_SIZE; ++i) {
        if (tmps->flag[i] == FALSE) {
            ++tmps->times;
            tmps->addrs[i] = addr;
            tmps->flag[i] = TRUE;
            return 0;
        }
    }
    return -1;
}
static int insert_tmp_from_back(TmpSegments* tmps, unsigned int addr)
{
    if (tmps->times >= TMP_SIZE) {
        return -1;
    }
    if (tmps->times == 0) {
        ++tmps->times;
        tmps->addrs[0] = addr;
        tmps->flag[0] = TRUE;
        return 0;
    }
    for (int i = TMP_SIZE - 2; i >= 0; --i) {
        if (tmps->flag[i] == TRUE && tmps->flag[i + i] == FALSE) {
            ++tmps->times;
            tmps->addrs[i + 1] = addr;
            tmps->flag[i + 1] = TRUE;
            return 0;
        }
    }
    printf("ERROR");
    exit(1);
    return -1;
}
static int check_tmp(TmpSegments* tmps)
{
    int times = 0;
    for (int i = 0; i < TMP_SIZE; ++i) {
        if (tmps->flag[i] == TRUE) {
            ++times;
        }
    }
    if (times == 1)
        return 0;
    return -1;
}
static int erase_tmp(TmpSegments* tmps, int index)
{
    if (index < 0 || index >= TMP_SIZE || tmps->flag[index] == FALSE) {
        return -1;
    }
    --tmps->times;
    tmps->flag[index] = FALSE;
    return 0;
}

void blks_sort(unsigned int start_addr, int offset, TmpSegments* tmps)
{
    Buffer buf;
    int times, next_addr, save_addr;
    unsigned char* blk = NULL;
    init_buf(&buf);
    init_tmp(tmps);
    while (start_addr) {
        /* read blk */
        read_blk(start_addr, &buf, &blk);
        times = convert(blk + 8 * 7);
        next_addr = convert(blk + 8 * 7 + 4);
        /* quick sort */
        blk_sort(blk, times, offset);
        /* save infomation of blk */
        save(blk + 8 * 7, times);
        save(blk + 8 * 7 + 4, 0);
        /* save as a tmp */
        save_addr = get_next_addr() + TMPBASE;
        write_blk(blk, save_addr, &buf);
        insert_tmp(tmps, save_addr);
        /* get next */
        start_addr = next_addr;
    }
}

static void adjust(int i, LoserTree* tree)
{
    int parent = (i + MERGE_N - 1) / 2;
    while (parent > 0) {
        if (tree->leaves[i] > tree->leaves[tree->loserTree[parent]]) {
            int temp = tree->loserTree[parent];
            tree->loserTree[parent] = i;
            /* i指向的是优胜者 */
            i = temp;
        }
        parent = parent / 2;
    }
    tree->loserTree[0] = i;
}

static void initLoserTree(LoserTree* tree, TmpSegments* tmps, Buffer* buf)
{
    int have;
    int index = 0;
    unsigned int addr;
    unsigned char* blk;
    for (int i = 0; i < MERGE_N; ++i) {
        have = 0;
        for (; index < TMP_SIZE; ++index) {
            if (tmps->flag[index] == TRUE && buf->numFreeBlk > 0) {
                have = 1;
                addr = tmps->addrs[index];
                printf("1___________________%d %d\n", index, addr);
                read_blk(addr, buf, &blk);
                printf("2___________________\n");

                tree->blk[i + 1] = blk;
                tree->offset[i + 1] = 0;
                tree->leaves[i + 1] = convert(blk);
                erase_tmp(tmps, index);
                ++index;
                break;
            }
        }
        if (!have)
            tree->leaves[i + 1] = TMPBASE;
    }
    tree->leaves[0] = -1;
    for (int i = 0; i < MERGE_N; i++)
        tree->loserTree[i] = 0;
    for (int i = MERGE_N; i > 0; i--)
        adjust(i, tree);
}

static int get_min_from_loser_tree(LoserTree* tree, Buffer* buf, int value[2])
{
    int index = tree->loserTree[0];
    int result = tree->leaves[index];
    if (result == TMPBASE)
        return result;
    value[0] = convert(tree->blk[index] + tree->offset[index] * 8);
    value[1] = convert(tree->blk[index] + tree->offset[index] * 8 + 4);
    int times = convert(tree->blk[index] + 7 * 8);
    int offset = ++tree->offset[index];
    if (offset < times) {
        tree->leaves[index] = convert(tree->blk[index] + tree->offset[index] * 8);
    } else {
        int addr = convert(tree->blk[index] + 7 * 8 + 4);
        printf("addr : %d\n", addr);
        if (addr == 0)
            tree->leaves[index] = TMPBASE;
        else {
            freeBlockInBuffer(tree->blk[index], buf);
            read_blk(addr, buf, &tree->blk[index]);
            tree->leaves[index] = convert(tree->blk[index]);
            tree->offset[index] = 0;
            // printf("value %d --------------------------\n", tree->leaves[index]);
        }
    }
    adjust(index, tree);
    return result;
}

void n_merge_sort(unsigned int start_addr, int offset)
{
    Buffer buf;
    TmpSegments tmps;
    LoserTree tree;
    int value[2];

    int r, in, loc;
    int index, tmp;
    int seg_start_addr, seg_now_addr, seg_next_addr;
    unsigned char *blk = NULL, *rblk = NULL;
    /* init */

    blks_sort(start_addr, offset, &tmps);

    // for (int i = 0; i < 255; ++i) {
    //     if (tmps.flag[i] == TRUE) {
    //         printf("______________________%d\n", i);

    //         read_data(tmps.addrs[i]);
    //     }
    // }
    /* use this to save result */
    while (1) {
        in = 0;
        loc = 0;

        seg_start_addr = get_next_addr() + TMPBASE;
        seg_now_addr = seg_start_addr;
        init_buf(&buf);
        rblk = getNewBlockInBuffer(&buf);

        initLoserTree(&tree, &tmps, &buf);

        while ((r = get_min_from_loser_tree(&tree, &buf, value)) != TMPBASE) {
            in = 1;
            if (loc >= 7) {
                loc = 0;
                seg_next_addr = get_next_addr() + TMPBASE;
                save(rblk + 7 * 8, 7);
                save(rblk + 7 * 8 + 4, seg_next_addr);
                write_blk(rblk, seg_now_addr, &buf);
                seg_now_addr = seg_next_addr;
                freeBlockInBuffer(rblk, &buf);
                rblk = getNewBlockInBuffer(&buf);
            }
            save(rblk + loc * 8, value[0]);
            save(rblk + loc * 8 + 4, value[1]);
            ++loc;
        }

        save(rblk + 7 * 8, 7);
        save(rblk + 7 * 8 + 4, 0);
        write_blk(rblk, seg_now_addr, &buf);
        freeBlockInBuffer(rblk, &buf);
        rblk = getNewBlockInBuffer(&buf);
        insert_tmp_from_back(&tmps, seg_start_addr);

        freeBuffer(&buf);
        if (check_tmp(&tmps) == 0) {
            read_data(seg_start_addr);

            printf("-----------\n");
            return;
        }
        // read_data(seg_start_addr);
    }
    read_data(seg_start_addr);
}
