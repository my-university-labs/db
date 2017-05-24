/* author: dongchangzhang */
/* time: Sat 20 May 2017 02:58:46 PM CST */
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

#define TMPBASE 400000

#include "extmem.h"

#define TRUE 1
#define FALSE 0

#define VALUE_BASE 0
#define SEARCH_BASE 100000
#define MERGE_BASE 200000
#define JOIN_BASE 300000
#define TMP_BASE 400000
#define ANY_BASE 500000
#define INDEX_BASE_R 600000
#define INDEX_BASE_S 700000

#define QUEUE_SIZE 1000

/* Queue */
struct Queue {
    int times, begin, end;
    unsigned int addrs[QUEUE_SIZE];
};
typedef struct Queue Queue;
void init_queue(Queue* q);
int pop_from_queue(Queue* q);
int push_into_queue(Queue* q, int addr);
// end here

/* Loser Tree (used by merge sort) */
struct LoserTree {
    int leaves[MERGE_N + 1];
    unsigned char* blk[MERGE_N + 1];
    short loserTree[MERGE_N], offset[MERGE_N + 1];
};
typedef struct LoserTree LoserTree;
void adjust(int i, LoserTree* tree);
void initLoserTree(LoserTree* tree, Queue* queue, Buffer* buf);
int get_min_from_loser_tree(LoserTree* tree, Buffer* buf, int value[2]);
// end here

/* Merge Sort */
int cmp_blk_item(const void* a, const void* b);
void blk_sort(unsigned char* blk, int times, int offset);
void blks_sort(unsigned int start_addr, int offset, Queue* queue);
int n_merge_sort(unsigned int start_addr, int offset);
//end here

/* Create Next Addr Randomly */
void clear_mark();
int get_next_addr(int base);
// end here

/* Create Data Randomly */
int get_a_data(int begin, int end);
// end here

/* Some Funcs For extmem.c */
void init_buf(Buffer* buf);
void write_blk(unsigned char* blk, unsigned int addr, Buffer* buf);
void read_blk(unsigned int addr, Buffer* buf, unsigned char** blk);
//end here

/* Tools */
void save(unsigned char* blk, int num);
int convert(unsigned char* blk);
int save_start_addr_into_file(char* rel, char* col1, char* col2, int start_addr);
int get_start_addr_from_file(char* rel, char* col, int r[]);
int compare(int value1, int value2, int op);
int link_addr(int addr1, int addr2);
// end here

/* Save Blk Into Disk */
void save_blk(Buffer* buf, unsigned char** des, unsigned char* from, int* index, int* save_to);
void save_last_blk(Buffer* buf, unsigned char* from, int times, int save_to);
// end here

/* Operation For Set */
int cmp_tuple(unsigned char* a, unsigned char* b, int offset);
void try_to_save_for_set(Buffer* buf, unsigned char** blk_saver, unsigned char* blk, int offset, int* index_saver, int* save_to, unsigned char last_insert[8]);
// end here

/* Read Blk Or Blks From Disk */
int read_a_data(int addr);
int read_data(int addr);
// end here

/* For Hash And Index */
int hash_function(int value);
int hash_index_base(char* rel);
void init_hash_index(char* rel);
void drop_hash_index(char* rel);
int create_hash_index(int addr, int offset, int key, char* rel);
// end here
#endif