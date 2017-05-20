/* author: dongchangzhang */
/* time: Sat 20 May 2017 02:58:46 PM CST */
#include "utils.h"
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
    memset(&tmps->flag, 0, sizeof(tmps->flag));
}
static int insert_tmp(TmpSegments* tmps, unsigned int addr)
{
    if (tmps->times >= 255) {
        return -1;
    }
    for (int i = 0; i < 255; ++i) {
        if (tmps->flag[i] == FALSE) {
            ++tmps->times;
            tmps->addrs[i] = addr;
            tmps->flag[i] = TRUE;
            return 0;
        }
    }
    return -1;
}
static int erase_tmp(TmpSegments* tmps, int index)
{
    if (index < 0 || index >= 255 || tmps->flag[index] == FALSE) {
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
// static void init_loser_tree(LoserTree* tree)
// {
//     for (int i = 0; i < MERGE_N; ++i) {
//         tree->flag[i] = FALSE;
//         tree->blk[i] = NULL;
//     }
// }
// static void loser_tree_arrange(LoserTree* tree)
// {

// }
// static void loser_tree_load(LoserTree* tree, TmpSegments* tmps, Buffer* buf)
// {
//     int index = 0;
//     unsigned int addr;
//     unsigned char* blk;
//     init_loser_tree(tree);
//     /* 初始化每一个叶子节点 */
//     for (int i = 0; i < MERGE_N; ++i) {
//         /* 对每一个已经排好序的段 */
//         for (; index < 255; ++index) {
//             /* 找到待排序的段 并且缓存有空间*/
//             if (tmps->flag[index] == TRUE && buf->numFreeBlk > 0) {
//                 /* 获取该段的地址 标记*/
//                 addr = tmps->addrs[index];
//                 tmps->flag[index] = FALSE;
//                 /* read this blk */
//                 blk = getNewBlockInBuffer(buf);
//                 read_blk(addr, buf, &blk);
//                 /* add this point into tree */
//                 tree->blk[i] = blk;
//                 tree->flag[i] = TRUE;
//                 tree->offset[i] = 0;
//                 tree->tree[MERGE_N +i] = i;
//             }
//         }
//     }
// }
// static void init_info(BufferInfo* info)
// {
//     int i;
//     info->times = 0;
//     for (i = 0; i < 7; ++i) {
//         info->pointers[i] = NULL;
//         info->addrs[i] = -1;
//         info->offsets[i] = 0;
//     }
// }
// static int get_new_info_index(BufferInfo* info)
// {
//     int i;
//     if (info->times >= 7)
//         return -1;
//     for (i = 0; i < 7 && info->pointers[i]; ++i) {
//         ;
//     }
//     return i;
// }
// static int get_next_addr_from_info(BufferInfo* info)
// {
//     int i, addr;
//     for (i = 0; i < 7 && info->addrs[i] == -1; ++i) {
//         ;
//     }
//     addr = info->addrs[i];
//     info->addrs[i] = -1;
//     return addr;
// }
// static void insert_info(BufferInfo* info, unsigned char* blk, unsigned int addr)
// {
//     int i = get_new_info_index(info);
//     ++info->times;
//     info->pointers[i] = blk;
//     info->offsets[i] = 0;
//     for (i = 0; i < 7 && info->addrs[i] != -1; ++i)
//         ;
//     info->addrs[i] = addr;
// }
// static void update_info(BufferInfo* info, int index)
// {
//     ++info->offsets[index];
// }
// static void check_info(Buffer* buf, BufferInfo* info)
// {
//     int i;
//     for (i = 0; i < 7; ++i) {
//         if (info->offsets[i] >= 7) {
//             // next_addr = get_next_addr_from_info(info);
//             // save(info->pointers[i] + 8 * 7 + 4, next_addr);
//             // write_blk(info->pointers[i], save_addr, buf);
//             freeBlockInBuffer(info->pointers[i], buf);
//             --info->times;
//             info->pointers[i] = NULL;
//             info->offsets[i] = 0;
//         }
//     }
// }
// static int get_min(BufferInfo* info, int offset, int* result)
// {
//     int i, tmp = -1, r = -1;
//     *result = MAX;
//     for (i = 0; i < 7; ++i) {
//         if (info->pointers[i]) {
//             memcpy(&tmp, info->pointers[i] + offset * sizeof(int), sizeof(int));
//             if (tmp < *result) {
//                 *result = tmp;
//                 r = i;
//             }
//         }
//     }
//     return r;
// }

// void loser_tree()
// {
//     Buffer buf;
//     init_buf(&buf);
// }
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

    int index = 0;
    unsigned int addr;
    unsigned char* blk;
    for (int i = 0; i < MERGE_N; ++i) {
        for (; index < 255; ++index) {
            if (tmps->flag[index] == TRUE && buf->numFreeBlk > 0) {
                addr = tmps->addrs[index];
                tmps->flag[index] = FALSE;
                read_blk(addr, buf, &blk);
                tree->blk[i] = blk;
                tree->flag[i] = TRUE;
                tree->offset[i] = 0;
                tree->leaves[i + 1] = convert(blk);
                ++index;
                break;
            }
        }
    }
    tree->leaves[0] = -1;
    for (int i = 0; i < MERGE_N; i++)
        tree->loserTree[i] = 0;
    for (int i = MERGE_N; i > 0; i--)
        adjust(i, tree);
}

static int get_min_from_loser_tree(LoserTree* tree, Buffer* buf)
{
    int index = tree->loserTree[0];
    int result = tree->leaves[index];
    int offset = ++tree->offset[index];
    if (offset != 7) {
        tree->leaves[index] = convert(tree->blk[index] + tree->offset[index] * 8);
    } else {
        tree->leaves[index] = TMPBASE;
    }
    adjust(index, tree);
    return result;
}
void n_merge_sort(unsigned int start_addr, int offset)
{
    Buffer buf;
    TmpSegments tmps;
    LoserTree tree;

    int index, tmp;
    int loc = 0;
    int times, next_addr = start_addr;
    unsigned char *blk = NULL, *rblk = NULL;
    /* init */
    init_buf(&buf);
    blks_sort(start_addr, offset, &tmps);
    initLoserTree(&tree, &tmps, &buf);
    /* use this to save result */
    rblk = getNewBlockInBuffer(&buf);
    int r;
    int j = 0;
    printf("_________________\n");
    while (1) {
        r = get_min_from_loser_tree(&tree, &buf);
        printf("%d - %d\n", r, ++j);
        if (r == TMPBASE)
            break;
    }
}
