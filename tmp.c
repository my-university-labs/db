
/* when find -> save it into there */
int next_addr, result_addr, result_now_addr, result_next_addr;
result_addr = get_next_addr() + MAX;
result_now_addr = result_addr;
/* init buffer */
if (!initBuffer(BUFFER_SIZE, BLOCK_SIZE, &buf)) {
    perror("Buffer Initialization Failed!\n");
    return -1;
}
if (!initBuffer(BUFFER_SIZE, BLOCK_SIZE, &buf_save)) {
    perror("Buffer Initialization Failed!\n");
    return -1;
}
/* get start addr of relation */
get_start_addr_from_file(rel, col, addr);
offset = addr[1];
next_addr = addr[0];

blk_save = getNewBlockInBuffer(&buf_save);
/* read blk from disk and save it into buffer */
while (next_addr) {
    printf("Search from blk %d\n", next_addr);

    if ((blk = readBlockFromDisk(next_addr, &buf)) == NULL) {
        perror("Reading Block Failed!\n");
        exit(0);
    }
    /* the number of tuple in this blk */
    memset(tmp, 0, 4);
    memcpy(tmp, blk + 8 * 7, 4);
    times = format_byte_to_int(tmp);
    printf("-%d\n", times);
    for (i = 0; i < times; ++i) {
        memset(tmp, 0, 4);
        memcpy(tmp, blk + 8 * 7 + 1 * 4, 4);
        data = format_byte_to_int(tmp);
        printf("%d\n", data);

        if (compare(data, value, op)) {
            /* data had been found save it */
            printf("-> Find it\n");

            if (r_times >= 7) {
                /* write result into disk */
                r_times = 0;

                format_int_to_byte(7, tmp);
                memcpy(blk_save + 7 * 8, tmp, 4);

                result_next_addr = get_next_addr() + MAX;

                format_int_to_byte(result_next_addr, tmp);
                memcpy(blk_save + 7 * 8 + 4, tmp, 4);
                printf("Write result into %d\n", result_now_addr);

                if (writeBlockToDisk(blk_save, result_now_addr, &buf_save) != 0) {
                    perror("Writing Block Failed!\n");
                    return -1;
                }
                result_now_addr = result_next_addr;

                freeBlockInBuffer(blk_save, &buf_save);
                blk_save = getNewBlockInBuffer(&buf_save);
            }
            memcpy(blk_save + r_times * 8, blk + 8 * i, 8);
            ++r_times;
        }
    }

    memset(tmp, 0, 4);
    memcpy(tmp, blk + 8 * 7 + 4, 4);
    next_addr = format_byte_to_int(tmp);
    /* free blk */
    freeBlockInBuffer(blk, &buf);
}

format_int_to_byte(r_times, tmp);
memcpy(blk_save + 7 * 8, tmp, 4);

format_int_to_byte(0, tmp);
memcpy(blk_save + 7 * 8 + 4, tmp, 4);
printf("Write result into %d\n", result_now_addr);
if (writeBlockToDisk(blk_save, result_now_addr, &buf_save) != 0) {
    perror("Writing Block Failed!\n");
    return -1;
}
printf("Search %s done!\n", rel);
freeBuffer(&buf);
freeBuffer(&buf_save);
