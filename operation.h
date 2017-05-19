/* author: dongchangzhang */
/* time: 2017年05月18日 星期四 16时48分58秒 */

#ifndef OPERATION_H_
#define OPERATION_H_

#include "extmem.h"

// size of buffer
#define BUFFER_SIZE 512 + 8
// size of block
#define BLOCK_SIZE 64

#define EQ 0 // ==
#define GT 1 // >
#define LT 2 // <
#define GE 3 // >=
#define LE 4 // <=

void init();
/* create data */
int create_data(char* rel, char* col1, int begin1, int end1, char* col2, int begin2, int end2, int times);

int read_data(int addr);

int get_start_addr_from_file(char* rel, char* col, int r[]);

int liner_search(char* rel, char* col, int op, int value);

int merge(int addr1, int addr2);

int combine(int addr1, int addr2);

int intersect(int addr1, int addr2);

int except(int addr1, int addr2);

#endif