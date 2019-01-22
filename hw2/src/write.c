
/*
 * Write out grade database to ASCII file.
 */

#include <stddef.h>
#include <stdio.h>
#include "global.h"
#include "gradedb.h"
#include "write.h"
/*
void test(Course *c){
    Professor *p;
    p = c->professor;
    //p = c->professor;
    printf("pointer of course : %p\n",(void *)c);
    printf("professor pointer is %p\n",(void *)p);
    printf(" surname pointer is %p\n",(void *)(p->surname));
    printf("character of surname : %c\n",(char)*(p->surname));
    printf(" ASSISTANT %s, %s\n", p->surname, p->name);

}
*/

void writeprofessor(fd, p)
FILE *fd;
Professor *p;
{
        //printf("write.c : writeprofessor :  \n");
        if(p == NULL) {
            //printf("If success\n");
            return;
        }
        //printf("If fail\n");
        //printf(" File is %p\n" , (void *)fd );
        //printf(" p is %p\n",(void *)p);
        //printf(" p is %p\n",(void *)(p->surname));
        //printf("character of surname : %c\n",(char)*(p->surname));
        /*if(p->surname == NULL){
            printf("surname is null\n");
        }*/
        //printf(" Surname is %s\n" , p->surname);
        //printf(" name is %s\n" , p->name);
        fprintf(fd, " PROFESSOR %s, %s\n", p->surname, p->name);
}

void writeassistant(fd, a)
FILE *fd;
Assistant *a;
{
        //printf("write.c : writeassistant :  \n");
        if(a == NULL) return;
        fprintf(fd, " ASSISTANT %s, %s\n", a->surname, a->name);
}

void writescore(fd, s)
FILE *fd;
Score *s;
{
        //printf("write.c : writescore : start \n");
        fprintf(fd, "   SCORE %s", s->asgt->name);
        if(s->flag == VALID) {
                //printf("write.c : writescore : in if \n");
                fprintf(fd, " %f", s->grade);
        } else {
                //printf("write.c : writescore : in if \n");
                switch(s->subst) {
                case USERAW:
                        fprintf(fd, " USERAW %f", s->grade);
                        break;
                case USENORM:
                        switch(s->asgt->npolicy) {
                        case QUANTILE:
                          fprintf(fd, " USENORM %f", s->qnorm);
                          break;
                        case LINEAR:
                          fprintf(fd, " USENORM %f", s->lnorm);
                          break;
                        case SCALE:
                          fprintf(fd, " USENORM %f", s->snorm);
                          break;
                        case RAW:
                          break;
                        }
                case USELIKEAVG:
                        fprintf(fd, " USELIKEAVG");
                        break;
                case USECLASSAVG:
                        fprintf(fd, " USECLASSAVG");
                        break;
                }
                fprintf(fd, " %s", s->code);
        }
        fprintf(fd, "\n");
        //printf("write.c : writescore : end \n");
}

void writestudent(fd, s)
FILE *fd;
Student *s;
{
        Score *sp;
        //printf("write.c : writestudent : start \n");
        fprintf(fd, "  STUDENT %s %s, %s\n", s->id, s->surname, s->name);
        for(sp = s->rawscores; sp != NULL; sp = sp->next)
                writescore(fd, sp);
        //printf("write.c : writestudent : end \n");
}

void writesection(fd, s)
FILE *fd;
Section *s;
{
        Student *sp;
        //printf("write.c : writesection : start \n");
        fprintf(fd, " SECTION %s\n", s->name);
        writeassistant(fd, s->assistant);
        for(sp = s->roster; sp != NULL; sp = sp->next)
                writestudent(fd, sp);
        //printf("write.c : writesection : end \n");
}

void writeassignment(fd, a)
FILE *fd;
Assignment *a;
{
        //printf("write.c : writeassignment : start \n");
        fprintf(fd, " ASSIGNMENT %s %s\n", a->name, a->atype);
        if(a->wpolicy == WEIGHT)
                fprintf(fd, "  WEIGHT %f\n", a->weight);
        if(a->npolicy != RAW) {
                switch(a->npolicy) {
                case LINEAR:
                        fprintf(fd, "  NORMALIZE LINEAR: %f, %f",
                               a->mean, a->stddev);
                        break;
                case QUANTILE:
                        fprintf(fd, "  NORMALIZE QUANTILE");
                        break;
                case SCALE:
                        fprintf(fd, "  NORMALIZE SCALE %f", a->scale);
                case RAW:
                        break;
                }
                fprintf(fd, " %s\n", a->ngroup == BYCLASS ? "BYCLASS": "BYSECTION");
        }
        if(a->max != 0) {
          fprintf(fd, "  MAXIMUM %f\n", a->max);
        }
        //printf("write.c : writeassignment : end \n");
}

void writecourse(fd, c)
FILE *fd;
Course *c;
{
        Assignment *ap;
        Section *sp;
        //printf("write.c : writecourse : start \n");
        fprintf(fd, "COURSE %s %s\n", c->number, c->title);
        writeprofessor(fd, c->professor);
        //printf("write.c : writecourse : writeprofessor done \n");
        for(ap = c->assignments; ap != NULL; ap = ap->next)
                writeassignment(fd, ap);
        //printf("write.c : writecourse : one for loop done \n");
        for(sp = c->sections; sp != NULL; sp = sp->next)
                writesection(fd, sp);

        //printf("write.c : writecourse : end \n");
}

void writefile(f, c)
Course *c;
char *f;
{
        FILE *fd;
        //printf("write.c : writefile : start \n");
        if((fd = fopen(f, "w")) == NULL) {
                error("Can't write file: %s\n", f);
                return;
        }
        writecourse(fd, c);
        fclose(fd);
        //printf("write.c : writefile : end \n");
}

