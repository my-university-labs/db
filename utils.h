#ifndef UTILS_H
#define UTILS_H

#define EQ 0 // ==
#define GT 1 // >
#define LT 2 // <
#define GE 3 // >=
#define LE 4 // <=

// size of buffer
#define BUFFER_SIZE 512 + 8
// size of block
#define BLOCK_SIZE 64

#define MERGE_N 7

#define MAX 100000

#define TMP_SIZE 1000
#define TMPBASE 500000

#include "extmem.h"

#define TRUE 1
#define FALSE 0

struct segment {
    int times;
    unsigned int addrs[TMP_SIZE];
    char flag[TMP_SIZE];
};

struct LoserTree {
    short loserTree[MERGE_N];
    int leaves[MERGE_N + 1];
    short offset[MERGE_N + 1];
    unsigned char* blk[MERGE_N + 1];
};

typedef struct segment TmpSegments;

typedef struct LoserTree LoserTree;

void clear_mark();

int convert(unsigned char* blk);

void save(unsigned char* blk, int num);

void init_buf(Buffer* buf);

void write_blk(unsigned char* blk, unsigned int addr, Buffer* buf);

void read_blk(unsigned int addr, Buffer* buf, unsigned char** blk);

int get_next_addr();

int get_a_data(int begin, int end);

int save_start_addr_into_file(char* rel, char* col1, char* col2, int start_addr);

int get_start_addr_from_file(char* rel, char* col, int r[]);

int compare(int value1, int value2, int op);

int link_addr(int addr1, int addr2);

void blk_sort(unsigned char* blk, int times, int offset);

void blks_sort(unsigned int start_addr, int offset, TmpSegments* tmps);

void n_merge_sort(unsigned int start_addr, int offset);

void loser_tree();
void test_loser_tree(int start);
#endif