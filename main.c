#include "extmem.h"
#include "operation.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define log

char *rrel = "R", *rcol1 = "A", *rcol2 = "B";
char *srel = "S", *scol1 = "C", *scol2 = "D";

void generate_data()
{
    int r, s;
    printf("Try to create data for R...\n");
    r = create_data(rrel, rcol1, 1, 40, rcol2, 1, 1000, 2);
    printf("Try to create data for S...\n");
    s = create_data(srel, scol1, 20, 60, scol2, 1, 1000, 4);
    printf("Done!\n");
}
void do_read()
{
    int addr[2];
    get_start_addr_from_file(rrel, rcol1, addr);
    read_data(addr[0]);
    get_start_addr_from_file(srel, scol1, addr);
    read_data(addr[0]);
}
void do_merge()
{
    int addr1, addr2, r1[2], r2[2];
    get_start_addr_from_file(rrel, rcol1, r1);
    get_start_addr_from_file(srel, scol1, r2);
    addr1 = n_merge_sort(r1[0], 0);
    addr2 = n_merge_sort(r2[0], 0);
    read_data(addr1);
    read_data(addr2);
}
void l_search()
{
    int r, s, addr;
    printf("Liner search for R.A = 40 or S.C = 60\n");
    r = liner_search(rrel, rcol1, EQ, 40);
    s = liner_search(srel, scol1, EQ, 60);
    addr = link_addr(r, s);
    printf("Result is :\n");
    read_data(addr);
    printf("Done!\n");
}
void b_search()
{
    int b1, b2, addr;
    printf("Binary search for R.A = 40 or S.C = 60\n");
    b1 = binary_search(rrel, rcol1, EQ, 40);
    b2 = binary_search(srel, scol1, EQ, 60);
    addr = link_addr(b1, b2);
    printf("Result is: \n");
    read_data(addr);
    printf("Done!\n");
}
void index_search()
{
    int what, addr;
    char buf[16];
    printf("Input which Table You Want To Search(S/R)?\n");
    scanf("%s", buf);
    printf("What You Want To Search?\n");
    scanf("%d", &what);
    addr = search_hash_index(buf, what);
    if (addr == -1) {
        printf("Index FIle For This Key Not Exists\n");
        return;
    }
    read_data(addr);
}
void p_action()
{
    int addr;
    printf("Project for R.A\n");
    addr = project(rrel, rcol1);
    printf("Result is: \n");
    read_data(addr);
    printf("Done!\n");
}
void join_action()
{
    int addr, r1[2], r2[2];
    get_start_addr_from_file(rrel, rcol1, r1);
    get_start_addr_from_file(srel, scol1, r2);
    addr = join(r1[0], r2[0]);
    read_data(addr);
}
void intersect_action()
{
    int addr, r1[2], r2[2];
    get_start_addr_from_file(rrel, rcol1, r1);
    get_start_addr_from_file(srel, scol1, r2);
    addr = intersect(r1[0], r2[0]);
    read_data(addr);
}
void except_action()
{
    int addr, r1[2], r2[2];
    get_start_addr_from_file(rrel, rcol1, r1);
    get_start_addr_from_file(srel, scol1, r2);
    addr = except(r1[0], r2[0]);
    read_data(addr);
}
void nest_connection()
{
    int addr, r1[2], r2[2];
    get_start_addr_from_file(rrel, rcol1, r1);
    get_start_addr_from_file(srel, scol1, r2);
    addr = nested_loop_join(r1[0], r2[0], 0, 0);
    read_data(addr);
}
void merge_connection()
{
    int addr, r1[2], r2[2];
    get_start_addr_from_file(rrel, rcol1, r1);
    get_start_addr_from_file(srel, scol1, r2);
    addr = sort_merge_join(r1[0], r2[0], 0, 0);
    read_data(addr);
}
void hash_connection()
{
    hash_join(rrel, srel);
}
void print_menu()
{
    printf("|------------------------ Menu ------------------------|\n");
    printf("|  1. Create Data                                      |\n");
    printf("|  2. Look Data                                        |\n");
    printf("|  3. Merge Sort                                       |\n");
    printf("|                ---- Search Option ----               |\n");
    printf("|  4. Liner Search                                     |\n");
    printf("|  5. Binary Search                                    |\n");
    printf("|  6. Index                                            |\n");
    printf("|                ---- Project Option ----              |\n");
    printf("|  7. Project                                          |\n");
    printf("|                 ---- Set Option ----                 |\n");
    printf("|  8. Union                                            |\n");
    printf("|  9. Intersect                                        |\n");
    printf("| 10. Except                                           |\n");
    printf("|                 ---- Connection ----                 |\n");
    printf("| 11. Nested Loop Join                                 |\n");
    printf("| 12. Sort Merge Join                                  |\n");
    printf("| 13. Hash Join                                        |\n");
    printf("|                    ---------------                   |\n");
    printf("|  0. END                                              |\n");
    printf("|------------------------------------------------------|\n");
    printf("Input You Choice Now: ");
}
int main(int argc, char** argv)
{
    init();
    int option, addr;
    int finished = 0;
    while (!finished) {
        print_menu();
        if (scanf("%d", &option) != 1) {
            while ((getchar()) != '\n') {
                printf("Input Error Try Again!\n");
                scanf("%d", &option);
            }
        }
        switch (option) {
        case 1:
            generate_data();
            break;
        case 2:
            do_read();
            break;
        case 3:
            // n_merge_sort();
            do_merge();
            break;
        case 4:
            l_search();
            break;
        case 5:
            b_search();
            break;
        case 6:
            index_search();
            break;
        case 7:
            p_action();
            break;
        case 8:
            join_action();
            break;
        case 9:
            intersect_action();
            break;
        case 10:
            except_action();
            break;
        case 11:
            nest_connection();
            break;
        case 12:
            merge_connection();
            break;
        case 13:
            hash_connection();
            break;
        case 0:
            finished = 1;
            break;
        default:
            printf("Bad Option!\n");
        }
    }
    return 0;
}
