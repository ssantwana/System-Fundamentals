#include <stdio.h>
#include "sfmm.h"
#include "debug.h"

int main(int argc, char const *argv[]) {
    sf_mem_init();

    //double* ptr = sf_malloc(sizeof(double));

    //*ptr = 320320320e-320;
    sf_malloc(sizeof(int));
    //printf("%f\n", *ptr);
    //sf_show_block_info((sf_block_info *)((void *)ptr - 8));
    printf("\n");
    debug("");
    //sf_show_blocks();
    sf_show_free_lists();

    //sf_free(ptr);
    //sf_show_free_lists();

    sf_mem_fini();

    return EXIT_SUCCESS;
}
