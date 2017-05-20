#include "extmem.h"
#include "operation.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv)
{
    init();
    printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Create Data Now! >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");

    char *rrel = "R", *rcol1 = "A", *rcol2 = "B";
    char *srel = "S", *scol1 = "C", *scol2 = "D";
    int r = create_data(rrel, rcol1, 1, 40, rcol2, 1, 1000, 2);
    int s = create_data(srel, scol1, 20, 60, scol2, 1, 1000, 4);
    read_data(r);
    read_data(s);

    printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Create Data Done! >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");

    int line_s_r = liner_search(rrel, rcol1, EQ, 40);

    int line_s_s = liner_search(srel, scol1, EQ, 60);

    read_data(line_s_r);

    read_data(line_s_s);

    int addr_merge = link_addr(line_s_r, line_s_s);

    read_data(addr_merge);

    printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Sort Blocks! >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");
    read_data(r);

    n_merge_sort(r, 0);
    // TmpSegments tmps;
    // blks_sort(r, 0, &tmps);
    // for (int i = 0; i < 255; ++i) {
    //     if (tmps.flag[i] == TRUE) {
    //         printf("______________________%d\n", i);

    //         read_data(tmps.addrs[i]);
    //     }
    // }
    // test_loser_tree(r);

    // read_data(r);

    return 0;
}
