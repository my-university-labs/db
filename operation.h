/* author: dongchangzhang */
/* time: 2017年05月18日 星期四 16时48分58秒 */

#ifndef OPERATION_H_
#define OPERATION_H_

typedef unsigned int u_int;

// size of buffer
#define BUFFER_SIZE 512 + 8
// size of block
#define BLOCK_SIZE 64

void init();
/* create data */
u_int create_data(char* rel, char* col1, u_int begin1, u_int end1, char* col2, u_int begin2, u_int end2, u_int times);

u_int read_data(u_int addr, int tuple);

int get_start_addr_from_file(char* rel, char* col1, char* col2, int r[]);

#endif