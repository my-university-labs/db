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
int get_next_addr(int base)
{
    int num;
    do {
        num = rand() % (MAX);
    } while (mark[num]);
    mark[num] = 1;
    return num + base;
}
// static int lala = 1;
// static int hoho = 0;
/* create data for tuple */
int get_a_data(int begin, int end)
{
    // if (hoho++ % 2 == 0)
    //     ++lala;
    // if (lala > 40)
    //     lala = 0;
    // return 20 + (lala);
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
    int i = 0;
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
        save_addr = get_next_addr(MERGE_BASE);
        write_blk(blk, save_addr, &buf);
        insert_tmp(tmps, save_addr);
        /* get next */
        start_addr = next_addr;
    }
    freeBuffer(&buf);
}

static void adjust(int i, LoserTree* tree)
{
    int index, flag = 0;
    unsigned char *a, *b;
    int parent = (i + MERGE_N - 1) / 2;
    while (parent > 0) {
        index = tree->loserTree[parent];
        if (index != 0 && i != 0) {
            a = tree->blk[i] + 8 * tree->offset[i];
            b = tree->blk[index] + 8 * tree->offset[index];
            int m = convert(a + 4);
            int n = convert(b + 4);
            flag = m > n;
        } else {
            flag = 0;
        }
        if (tree->leaves[i] > tree->leaves[index] || (tree->leaves[i] == tree->leaves[index] && flag)) {
            int temp = tree->loserTree[parent];
            tree->loserTree[parent] = i;
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
                read_blk(addr, buf, &blk);
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
        if (addr == 0)
            tree->leaves[index] = TMPBASE;
        else {
            freeBlockInBuffer(tree->blk[index], buf);
            read_blk(addr, buf, &tree->blk[index]);
            tree->leaves[index] = convert(tree->blk[index]);
            tree->offset[index] = 0;
        }
    }
    adjust(index, tree);
    return result;
}

int n_merge_sort(unsigned int start_addr, int offset)
{
    Buffer buf;
    TmpSegments tmps;
    LoserTree tree;
    int value[2];

    int start_save_index = 0;
    char filename[64];
    FILE* fp;

    int r, in, loc;
    int index, tmp;
    int seg_start_addr, seg_now_addr, seg_next_addr;
    unsigned char *blk = NULL, *rblk = NULL;
    blks_sort(start_addr, offset, &tmps);
    while (1) {
        in = 0;
        loc = 0;

        seg_start_addr = get_next_addr(MERGE_BASE);
        seg_now_addr = seg_start_addr;
        init_buf(&buf);
        rblk = getNewBlockInBuffer(&buf);

        initLoserTree(&tree, &tmps, &buf);

        if (tmps.times == 0) {
            start_save_index = 1;
            sprintf(filename, "blk/index%d.blk", seg_now_addr);
            fp = fopen(filename, "w");
        }

        while ((r = get_min_from_loser_tree(&tree, &buf, value)) != TMPBASE) {
            in = 1;
            if (loc >= 7) {
                loc = 0;
                seg_next_addr = get_next_addr(MERGE_BASE);
                save(rblk + 7 * 8, 7);
                save(rblk + 7 * 8 + 4, seg_next_addr);
                write_blk(rblk, seg_now_addr, &buf);
                if (start_save_index)
                    fprintf(fp, "%d\n", seg_now_addr);
                seg_now_addr = seg_next_addr;
                rblk = getNewBlockInBuffer(&buf);
            }
            save(rblk + loc * 8, value[0]);
            save(rblk + loc * 8 + 4, value[1]);
            ++loc;
        }

        save(rblk + 7 * 8, 7);
        save(rblk + 7 * 8 + 4, 0);
        write_blk(rblk, seg_now_addr, &buf);
        if (start_save_index) {
            fprintf(fp, "%d\n", seg_now_addr);
            fclose(fp);
        }
        rblk = getNewBlockInBuffer(&buf);
        insert_tmp_from_back(&tmps, seg_start_addr);
        freeBuffer(&buf);
        if (tmps.times == 1)
            break;
    }

    return seg_start_addr;
}
void save_blk(Buffer* buf, unsigned char** des, unsigned char* from, int* index, int* save_to)
{
    int next;
    if (*index >= 7) {
        *index = 0;
        save(*des + 8 * 7, 7);
        next = get_next_addr(ANY_BASE);
        save(*des + 7 * 8 + 4, next);
        write_blk(*des, *save_to, buf);
        *save_to = next;
        *des = getNewBlockInBuffer(buf);
    }
    memcpy(*des + *index * 8, from, 8);
    ++(*index);
}
void save_last_blk(Buffer* buf, unsigned char* from, int times, int save_to)
{
    printf("--- %d %d\n", times, save_to);

    save(from + 7 * 8, times);
    save(from + 7 * 8 + 4, 0);
    write_blk(from, save_to, buf);
}
int cmp_tuple(unsigned char* a, unsigned char* b, int offset)
{
    int v11, v12;
    int v21, v22;
    memcpy(&v11, a + offset * 4, sizeof(int));
    memcpy(&v12, a + (1 - offset) * 4, sizeof(int));
    memcpy(&v21, b + offset * 4, sizeof(int));
    memcpy(&v22, b + (1 - offset) * 4, sizeof(int));
    if (v11 == v21 && v12 == v22) {
        return 0;
    } else if (v11 < v21 || (v11 == v21 && v12 < v22)) {
        return -1;
    } else {
        return 1;
    }
}
void try_to_save_for_set(Buffer* buf, unsigned char** blk_saver, unsigned char* blk, int offset, int* index_saver, int* save_to, unsigned char last_insert[8])
{
    if (memcmp(blk + offset * 8, last_insert, 8) != 0) {
        save_blk(buf, blk_saver, blk + offset * 8, index_saver, save_to);
        memcpy(last_insert, blk + offset * 8, 8);
    }
}

int hash_function(int value)
{
    return value;
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

static char hash_index_marker[MAX];

void init_hash_index(char* rel)
{
    int i;
    for (i = 0; i < 100; ++i) {
        hash_index_marker[i] = TRUE;
    }
    for (i = 100; i < MAX; ++i) {
        hash_index_marker[i] = FALSE;
    }
    drop_hash_index(rel);
}
static int get_new_blk_for_index()
{
    int i;
    for (i = 100; i < MAX; ++i) {
        if (hash_index_marker[i] == FALSE) {
            hash_index_marker[i] = TRUE;
            return i;
        }
    }
    return -1;
}
void drop_hash_index(char* rel)
{
    int i;
    char filename[128];
    for (i = 0; i < 100; ++i) {
        sprintf(filename, "blk/%d.blk", hash_index_base(rel) + i);
        if (remove(filename) == 0) {
            printf("Remove Old Index File %s\n", filename);
        }
    }
}

int hash_index_base(char* rel)
{
    return (strcmp(rel, "R") == 0 ? INDEX_BASE_R : INDEX_BASE_S);
}
int create_hash_index(int addr, int offset, int key, char* rel)
{
    FILE* fp_try;
    char filename[128];

    Buffer buf;
    unsigned char* blk;
    int have = 0, times, now, next;

    now = hash_index_base(rel) + hash_function(key);

    sprintf(filename, "blk/%d.blk", now);
    if (!(fp_try = fopen(filename, "r"))) {
        have = 0;
    } else {
        have = 1;
        fclose(fp_try);
    }

    init_buf(&buf);

    if (have) {
        read_blk(now, &buf, &blk);
        times = convert(blk + 7 * 8);
        next = convert(blk + 7 * 8 + 4);
    } else {
        blk = getNewBlockInBuffer(&buf);
        times = 0;
        next = 0;
    }
    while (1) {
        if (times < 7) {
            save(blk + times * 8, addr);
            save(blk + times * 8 + 4, offset);
            ++times;
            save(blk + 7 * 8, times);
            save(blk + 7 * 8 + 4, 0);
            write_blk(blk, now, &buf);
            break;
        } else {
            if (next == 0) {
                next = hash_index_base(rel) + get_new_blk_for_index();
                save(blk + 7 * 8 + 4, next);
                write_blk(blk, now, &buf);
                blk = getNewBlockInBuffer(&buf);

                times = 0;
                now = next;
                next = 0;
            } else {
                freeBlockInBuffer(blk, &buf);
                read_blk(now, &buf, &blk);
                now = next;
                times = convert(blk + 7 * 8);
                next = convert(blk + 7 * 8 + 4);
            }
        }
    }
    freeBuffer(&buf);
    return 0;
}
