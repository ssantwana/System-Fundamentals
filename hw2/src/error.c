/*
 * Error handling routines
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

int errors;
int warnings;
int dbflag = 1;

/*
void fatal(fmt, a1, a2, a3, a4, a5, a6)
char *fmt, *a1, *a2, *a3, *a4, *a5, *a6;
{
        fprintf(stderr, "\nFatal error: ");
        fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
        fprintf(stderr, "\n");
        exit(1);
}
*/
void fatal(char *fmt, ...)
{
        va_list argp;
        fprintf(stderr, "\nFatal error: ");
        va_start(argp, fmt);
        vfprintf(stderr, fmt, argp);
        va_end(argp);
        fprintf(stderr, "\n");
        exit(1);
}
/*
void error(fmt, a1, a2, a3, a4, a5, a6)
char *fmt, *a1, *a2, *a3, *a4, *a5, *a6;
{
        fprintf(stderr, "\nError: ");
        fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
        fprintf(stderr, "\n");
        errors++;
}
*/
void error(char *fmt, ...)
{
        va_list argp;
        fprintf(stderr, "\nError: ");
        va_start(argp, fmt);
        vfprintf(stderr, fmt, argp);
        va_end(argp);
        fprintf(stderr, "\n");
        errors++;
}

//void warning(fmt, a1, a2, a3, a4, a5, a6)
//char *fmt, *a1, *a2, *a3, *a4, *a5, *a6;
void warning(char *fmt, ...)
{


        va_list argp;
        fprintf(stderr, "\nWarning: ");
        va_start(argp, fmt);
        vfprintf(stderr, fmt, argp);
        va_end(argp);
        fprintf(stderr, "\n");

        /*
        fprintf(stderr, "\nWarning: ");
        fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
        //fprintf(stderr);// fmt, a1, a2, a3, a4, a5, a6);
        fprintf(stderr, "\n");
        */
        warnings++;
}



void debug(char *fmt, ...)
{
        if(!dbflag) return;
        printf("\n\n\n In Debug\n\n\n\n\n\n");
        va_list argp;
        fprintf(stderr, "\nDebug: ");
        va_start(argp, fmt);
        vfprintf(stderr, fmt, argp);
        va_end(argp);
        fprintf(stderr, "\n");

}

/*
void debug(fmt, a1, a2, a3, a4, a5, a6)
//char *fmt, *a1, *a2, *a3, *a4, *a5, *a6;
{

        if(!dbflag) return;





        fprintf(stderr, "\nDebug: ");
        fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
        fprintf(stderr, "\n");
}

*/