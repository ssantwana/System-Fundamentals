#include <stdlib.h>

#include "hw1.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    global_options = 0;
    if(!validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    debug("Options: 0x%lX", global_options);
    if(global_options & 0x1L<<63) {
        USAGE(*argv, EXIT_SUCCESS);
    }
    //global_options_in_hex(global_options)
    //printf("Global args : %lx\n",global_options);

    //recode()
    if(recode(argv) == 0){
        debug("recode failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */