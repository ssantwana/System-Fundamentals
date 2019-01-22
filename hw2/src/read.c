
/*
 * Read in grade database from ASCII files
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "global.h"
#include "gradedb.h"
#include "stats.h"
#include "allocate.h"
#include "read.h"
#include <string.h>
#include "debug.h"
/*
 * Input file stack
 */

Ifile *ifile;

/*
 * Token readahead buffer
 */
#define  LIMIT  200
char tokenbuf[LIMIT];
char *tokenptr = tokenbuf;
char *tokenend = tokenbuf;

Course *readfile(root)
char *root;
{
        Course *c;
        //Professor *p;

        ifile = newifile();
        ifile->prev = NULL;
        ifile->name = root;
        ifile->line = 1;
        //printf("read.c : Readfile : \n");
        debug("Testing %s","Devesh");
        if((ifile->fd = fopen(root, "r")) == NULL)
                fatal("Can't open data file %s.\n", root);

        fprintf(stderr, "[ %s", root);
        gobbleblanklines();
        c = readcourse();
        gobbleblanklines();
        expecteof();
        //printf("read.c : Readfile : About to close file \n");
        fclose(ifile->fd);
        //free()
        //printf("read.c : Readfile : After close file \n");


        //p = c->professor;
        //printf(" p is %p\n",(void *)p);
        //printf(" p is %p\n",(void *)(p->surname));
        //printf("character of surname : %c\n",(char)*(p->surname));
        ////printf(" ASSISTANT %s, %s\n", p->surname, p->name);




        fprintf(stderr, " ]\n");
        free(ifile);
        return(c);
}

Course *readcourse()
{
        Course *c;
        expecttoken("COURSE");
        //printf("read.c : reascourse : start \n");
        c = newcourse();
        c->number = readid();
        c->title = readname();
        c->professor = readprofessor();
        c->assignments = readassignments();
        c->sections = readsections(c->assignments);
        //printf("read.c : reascourse : end\n");
        return(c);
}

Professor *readprofessor()
{
        Professor *p;
        //printf("read.c : readprofessor : start\n");
        if(!checktoken("PROFESSOR")) return(NULL);
        p = newprofessor();
        p->surname = readsurname();
        p->name = readname();
        //printf("read.c : readprofessor : end\n");
        return(p);
}

Assistant *readassistant()
{
        Assistant *a;
        //printf("read.c : readassistant : start\n");
        if(!checktoken("ASSISTANT")) return(NULL);
        a = newassistant();
        a->surname = readsurname();
        a->name = readname();
        //printf("read.c : readassistant : end\n");
        return(a);
}

Assignment *readassignments()
{
        Assignment *a;
        //printf("read.c : readassignments : start\n");
        if(!checktoken("ASSIGNMENT")) return(NULL);
        a = newassignment();
        a->name = readid();
        a->atype = readatype();
        expectnewline();
        a->wpolicy = NOWEIGHT;
        a->npolicy = RAW;
        a->weight = 0.0;
        a->max = 0.0;
        a->ngroup = BYCLASS;
        do {
                if(checktoken("WEIGHT")) {
                        readweight(a);
                        expectnewline();
                } else if(checktoken("NORMALIZE")) {
                        readnorm(a);
                        expectnewline();
                } else if(checktoken("MAXIMUM")) {
                        readmax(a);
                        expectnewline();
                } else break;
        } while(TRUE);
        a->next = readassignments();

        //printf("read.c : readassignments : end\n");
        return(a);
}

Section *readsections(a)
Assignment *a;
{
        Section *s;
        //printf("read.c : readsections : start\n");
        while(!istoken()) {
                advancetoken();
                if(!istoken()) {
                        if(ifile->prev == NULL) return(NULL);
                        else previousfile();
                }
        }
        if(checktoken("FILE")) {
                pushfile();

                return(readsections(a));
        }
        if(!checktoken("SECTION")) return(NULL);
        s = newsection();
        s->name = readname();
        s->assistant = readassistant();
        s->roster = readstudents(a, s);
        s->next = readsections(a);

        //printf("read.c : readsections : end\n");
        return(s);
}

Student *readstudents(a, sep)
Assignment *a;
Section *sep;
{
        Student *s;
        //printf("read.c : readstudents : start\n");
        if(!checktoken("STUDENT")) return(NULL);
        s = newstudent();
        s->id = readid();
        s->surname = readsurname();
        s->name = readname();
        s->rawscores = readscores(a);
        s->section = sep;
        s->next = readstudents(a, sep);
        //printf("read.c : readstudents : end\n");

        return(s);
}

Score *readscores(a)
Assignment *a;
{
        Score *s;
        Assignment *ap;
        //printf("read.c : read scores : \n");
        if(!checktoken("SCORE")) return(NULL);
        s = newscore();
        if(!istoken()) advancetoken();
        s->asgt = NULL;
        /*
         * Failure to read assignment ID has to be a fatal error,
         * because other code depends on a valid s->asgt pointer.
         */
        if(istoken()) {
                for(ap = a; ap != NULL; ap = ap->next) {
                        if(!strcmp(ap->name, tokenptr)) {
                                s->asgt = ap;
                                break;
                        }
                }
                if(s->asgt == NULL)
                   fatal("(%s:%d) Undeclared assignment %s encountered in student scores.",
                         ifile->name, ifile->line, tokenptr);
                flushtoken();
        } else {
                fatal("(%s:%d) Expecting assignment ID.", ifile->name, ifile->line);
        }
        readgrade(s);
        expectnewline();
        s->next = readscores(a);

        //printf("read.c : read scores : end\n");
        return(s);
}

void readgrade(s)
Score *s;
{
        float f;
        //printf("read.c : readgrades :start \n");
        s->flag = INVALID;
        if(checktoken("USERAW")) s->subst = USERAW;
        else if(checktoken("USENORM")) s->subst = USENORM;
        else if(checktoken("USELIKEAVG")) s->subst = USELIKEAVG;
        else if(checktoken("USECLASSAVG")) s->subst = USECLASSAVG;
        else s->flag = VALID;

        if(s->flag == VALID) {
                if(!istoken()) advancetoken();
                if(istoken() && (sscanf(tokenptr, "%f", &f) == 1)) {
                        flushtoken();
                        s->grade = f;
                } else {
                        error("(%s:%d) Expected a numeric score.", ifile->name, ifile->line);
                        s->grade = 0.0;
                }
                if(s->asgt->max != 0.0 && s->grade > s->asgt->max)
                  warning("(%s:%d) Grade (%f) exceeds declared maximum value (%f).\n",
                          ifile->name, ifile->line, s->grade, s->asgt->max);
        } else {
                switch(s->subst) {
                case USERAW:
                        if(!istoken()) advancetoken();
                        if(istoken() && (sscanf(tokenptr, "%f", &f) == 1)) {
                                flushtoken();
                                s->grade = f;
                        } else {
                                error("(%s:%d) Expected a numeric value.", ifile->name, ifile->line);
                                s->grade = 0.0;
                        }
                        if(s->asgt->max != 0.0 && s->grade > s->asgt->max)
                          warning("(%s:%d) Grade (%f) exceeds declared maximum value (%f).\n",
                                  ifile->name, ifile->line, s->grade, s->asgt->max);
                        break;
                case USENORM:
                        if(!istoken()) advancetoken();
                        if(istoken() && (sscanf(tokenptr, "%f", &f) == 1)) {
                          flushtoken();
                          s->qnorm = s->lnorm = s->snorm = f;
                          switch(s->asgt->npolicy) {
                          case RAW:
                            break;
                          case LINEAR:
                            if (s->lnorm < s->asgt->mean - 4.0*s->asgt->stddev ||
                                s->lnorm > s->asgt->mean + 4.0*s->asgt->stddev)
                              warning("(%s:%d) LINEAR score (%f) seems strange.\n",
                                      ifile->name, ifile->line, s->lnorm);
                            break;
                          case QUANTILE:
                            if(s->qnorm < 0.0 || s->qnorm > 100.0) {
                              error("(%s:%d) QUANTILE score (%f) not in [0.0, 100.0]\n",
                                    ifile->name, ifile->line, s->qnorm);
                            }
                            break;
                          case SCALE:
                            if(s->snorm < 0.0 || s->snorm > s->asgt->scale) {
                              error("(%s:%d) SCALE score (%f) not in [0.0, %f]\n",
                                    ifile->name, ifile->line, s->snorm, s->asgt->scale);
                            }
                            break;
                          }
                        } else {
                                error("(%s:%d) Expected a normalized score.", ifile->name, ifile->line);
                                s->qnorm = s->lnorm = s->snorm = 0.0;
                        }
                        break;
                case USELIKEAVG:
                case USECLASSAVG:
                        break;
                }
                advanceeol();
                s->code = newstring(tokenptr, tokensize());
                flushtoken();
        }

        //printf("read.c : readgrades :end \n");
}

void readweight(a)
Assignment *a;
{
        float f;
        //printf("read.c : readweight : start\n");
        advancetoken();
        if(istoken() && sscanf(tokenptr, "%f", &f) == 1) {
                flushtoken();
                a->wpolicy = WEIGHT;
                a->weight = f;
        } else {
                error("(%s:%d) Expected a numeric weight.", ifile->name, ifile->line);
                a->wpolicy = NOWEIGHT;
        }

        //printf("read.c : readweight : end\n");
}

void readmax(a)
Assignment *a;
{
        float f;
        //printf("read.c : readmax : start\n");
        advancetoken();
        if(istoken() && sscanf(tokenptr, "%f", &f) == 1) {
                flushtoken();
                a->max = f;
        } else {
                error("(%s:%d) Expected a numeric maximum score.", ifile->name, ifile->line);
        }

        //printf("read.c : readmax : end\n");
}

void readnorm(a)
Assignment *a;
{
        float f;
        int found;
        //printf("read.c : readnorm : start\n");
        found = 0;
        do {
                if(checktoken("RAW")) {
                        a->npolicy = RAW;
                } else if(checktoken("STDLINEAR")) {
                        a->npolicy = LINEAR;
                        a->mean = 0.0;
                        a->stddev = 1.0;
                } else if(checktoken("GENLINEAR")) {
                        a->npolicy = LINEAR;
                        advancetoken();
                        if(istoken() && sscanf(tokenptr, "%f", &f) == 1) {
                                flushtoken();
                                a->mean = f;
                        } else {
                                error("(%s:%d) Expected a numeric mean, using 0.0.", ifile->name, ifile->line);
                                a->mean = 0.0;
                        }
                        advancetoken();
                        if(istoken() && sscanf(tokenptr, "%f", &f) == 1) {
                                flushtoken();
                                a->stddev = f;
                        } else {
                                error("(%s:%d) Expected a numeric standard deviation, using 1.0.", ifile->name, ifile->line);
                                a->stddev = 1.0;
                        }
                } else if(checktoken("QUANTILE")) {
                        a->npolicy = QUANTILE;
                } else if(checktoken("SCALE")) {
                        a->npolicy = SCALE;
                        advancetoken();
                        if(istoken() && sscanf(tokenptr, "%f", &f) == 1) {
                                flushtoken();
                                a->scale = f;
                        } else {
                                error("(%s:%d) Expected a numeric scale, using 0.0.", ifile->name, ifile->line);
                                a->scale = 0.0;
                        }
                } else if(checktoken("BYCLASS")) {
                        a->ngroup = BYCLASS;
                } else if(checktoken("BYSECTION")) {
                        a->ngroup = BYSECTION;
                } else break;
                found++;
        } while(TRUE);
        if(!found) {
                a->npolicy = RAW;
                a->ngroup = BYCLASS;
                error("(%s:%d) Expected normalization information.", ifile->name, ifile->line);
        }

        //printf("read.c : readnorm : end\n");
}

Surname readsurname()
{
        Surname s;
        //printf("read.c : readsurname : start\n");
        if(!istoken()) advancetoken();
        if(istoken()) s = newstring(tokenptr, tokensize());
        else {
                error("(%s:%d) Expected surname.", ifile->name, ifile->line);
                s = newstring("", 0);
        }
        flushtoken();

        //printf("read.c : readsurname : end\n");
        return(s);
}

Name readname()
{
        Name n;
        //printf("read.c : readname : start \n");
        advanceeol();
        if(istoken()) n = newstring(tokenptr, tokensize());
        else {
                error("(%s:%d) Expected a name.", ifile->name, ifile->line);
                n = newstring("", 0);
        }
        flushtoken();
        expectnewline();
        //printf("read.c : readname : end\n");
        return(n);
}

Id readid()
{
        Id i;
        //printf("read.c : readid : start\n");
        if(!istoken()) advancetoken();
        if(istoken()) i = newstring(tokenptr, tokensize());
        else {
                error("(%s:%d) Expected an ID.", ifile->name, ifile->line);
                i = newstring("", 0);
        }
        flushtoken();

        //printf("read.c : readid : end\n");

        return(i);
}

Atype readatype()
{
        Atype a;
        //printf("read.c : readtype : start\n");
        if(!istoken()) advancetoken();
        if(istoken()) a = newstring(tokenptr, tokensize());
        else {
                error("(%s:%d) Expected an assignment type.", ifile->name, ifile->line);
                a = newstring("", 0);
        }
        flushtoken();

        //printf("read.c : readtype : end\n");
        return(a);
}

/*
 * See if there is read ahead in the token buffer
 */

int istoken()
{
        //printf("read.c : istoken : \n");
        //printf("tokenptr : %p tokenend : %p \n",tokenptr,tokenend);
        if(tokenptr != tokenend){return(TRUE);
        }
        else{
            //printf("About to return false\n");
            return(FALSE);
        }
}

/*
 * Determine the size of the token in the buffer
 */

int tokensize()
{
        //printf("read.c : tokenize : \n");
        return(tokenend-tokenptr);
}

/*
 * Flush the token readahead buffer
 */

void flushtoken()
{
        //printf("read.c : flushtoken : \n");
        tokenptr = tokenend = tokenbuf;
}

int iswhitespace(c)
char c;
{
        //printf("read.c : iswhitespace : \n");
        if(c == ',' || c == ':' || c == ' ' ||
                           c == '\t' || c == '\f') return(TRUE);
        else return(FALSE);
}

void gobblewhitespace()
{

        char c;
        //printf("read.c : gobblewhitespace : start\n");
        if(istoken()) return;
        //printf("read.c : gobblewhitespace : before while\n");
        while(iswhitespace(c = getc(ifile->fd)));
        //printf("read.c : gobblewhitespace : after while\n");
        ungetc(c, ifile->fd);
        //printf("read.c : gobblewhitespace : after ungetc \n");

        //printf("read.c : gobblewhitespace : end\n");
}

void gobbleblanklines()
{
        char c;
        //printf("read.c : gobbleblanklines : start\n");
        if(istoken()) return;
        do {
          if((c = getc(ifile->fd)) == '#') {
            while((c = getc(ifile->fd)) != '\n') {
              if(c == EOF)
                fatal("(%s:%d) EOF within comment line.",
                      ifile->name, ifile->line);
            }
            ifile->line++;
            continue;
          }
          ungetc(c, ifile->fd);
          while((c = getc(ifile->fd)) == '\n') {
            ifile->line++;
            gobblewhitespace();
            break;
          }
          if(c == '\n') continue;
          ungetc(c, ifile->fd);
          return;
        } while(1);

        //printf("read.c : gobbleblanklines : end\n");
}

/*
 * Return the next character, either from the token readahead buffer,
 * or else from the input stream.  If we see EOF, it's an error.
 */

char nextchar()
{
        char c;
        //printf("read.c : nextchar : start\n");
        if(istoken()) return(*tokenptr++);
        flushtoken();
        if((c = getc(ifile->fd)) == EOF)
           fatal("(%s:%d) Unexpected EOF.", ifile->name, ifile->line);

        //printf("read.c : nextchar : end \n");
        return(c);
}

/*
 * Read the next input token into the token readahead buffer.
 * If we already have a partial token in the buffer, it is an error
 */

void advancetoken()
{
        char c;
        //printf("read.c : advancetoken : start\n");
        if(istoken()) {
            error("(%s:%d) Flushing unread input token.", ifile->name, ifile->line);
        }
        flushtoken();
        gobblewhitespace();
        while((c = getc(ifile->fd)) != EOF) {
                if(iswhitespace(c) || c == '\n') {
                        ungetc(c, ifile->fd);
                        break;
                }
                if(tokenend >= (tokenbuf + LIMIT))
                    continue;
                *tokenend++ = c;
        }
        if(tokenend != tokenptr && tokenend < (tokenbuf + LIMIT)) *tokenend++ = '\0';

        //printf("read.c : advancetoken : end\n");
}

/*
 * Read from the current position to the end of the line into the
 * token buffer.  If we already had a token in the buffer, it's an error.
 */

void advanceeol()
{
        char c;
        //printf("read.c : advanceeol : start\n");
        if(istoken()){
            error("(%s:%d) Flushing unread input token.", ifile->name, ifile->line);
        }

        flushtoken();
        gobblewhitespace();
        while((c = getc(ifile->fd)) != EOF) {
                //printf("while in %c \n",c);
                if(c == '\n') {
                        ungetc(c, ifile->fd);
                        break;
                }
                //printf("while out %c\n",c);
                //printf(" tokenend : %p tokenbuf : %p\n",tokenend,(tokenbuf + 32));
                if(tokenend >= (tokenbuf + LIMIT))
                    continue;
                *tokenend++ = c;
        }
        //printf("read.c : advanceeol : while finished\n");
        if(c == EOF)
                fatal("(%s:%d) Incomplete line at end of file.", ifile->name, ifile->line);
        if(tokenend < (tokenbuf + LIMIT))
            *tokenend++ = '\0';

        //printf("read.c : advanceeol : end\n");

}

/*
 * Check to see that the next token in the input matches a given keyword.
 * If it does not match, it is an error, and the input is left unchanged.
 * If it does match, the matched token is removed from the input stream.
 */

void expecttoken(key)
char *key;
{
        //printf("read.c : expecttoken : start\n");
        if(!istoken()) advancetoken();
        if(istoken() && !strcmp(tokenptr, key)) {
                //printf("here\n");
                flushtoken();
        } else {
                error("(%s:%d) Expected %s, found %s", ifile->name, ifile->line, key, tokenptr);
        }
        //printf("read.c : expecttoken : end\n");
}

/*
 * Check to see that the next token in the input matches a given keyword.
 * If it does not match, FALSE is returned, and the input is unchanged.
 * If it does match, the matched token is removed from the input stream,
 * and TRUE is returned.
 */

int checktoken(key)
char *key;
{
        //printf("read.c : checktoken : \n");
        if(!istoken()) advancetoken();
        if(istoken() && !strcmp(tokenptr, key)) {
                flushtoken();
                return(TRUE);
        } else {
                return(FALSE);
        }
}

void expectnewline()
{
        char c;
        //printf("read.c : expectnewline : start\n");
        gobblewhitespace();
        if((c = nextchar()) == '\n') {
          ifile->line++;
          gobbleblanklines();
          return;
        }
        else {
                error("(%s:%d) Expected newline, scanning ahead.", ifile->name, ifile->line);
                flushtoken();
                while((c = nextchar()) != '\n');
                ifile->line++;
        }
        //printf("read.c : expectnewline : end\n");
}

void expecteof()
{
        char c;
        //int x = 0;
        //printf("read.c : expecteof :start \n");
        /*
        if((c = getc(ifile->fd)) == EOF){
            printf("***Watch***\n");
        }
        if( (c = getc(ifile->fd)) == EOF )
            x = 1;
        printf("Phase 1 passed\n");
        */

        if(!istoken() &&  (c = getc(ifile->fd)) == EOF && ifile->prev == NULL){
           // printf("In If\n");
           return;
        }
        else {
                //printf("read.c : expecteof : inside else \n");
                error("(%s:%d) Expected EOF, skipping excess input.", ifile->name, ifile->line);
                flushtoken();
                while(ifile->prev != NULL) previousfile();
        }

        //printf("read.c : expecteof :end \n");
}

void previousfile()
{
        Ifile *prev;
        //printf("read.c : previousfile : start\n");
        if((prev = ifile->prev) == NULL)
                fatal("(%s:%d) No previous file.", ifile->name, ifile->line);
        //printf("read.c : previousfile : before free\n");
        fclose(ifile->fd);
        free(ifile);
        //printf("read.c : previousfile : after free\n");
        //printf("read.c : previousfile : ifile pointer %p \n",(void *)ifile);
        //printf("read.c : previousfile : file pointer %p \n",(void *)ifile->fd);

        //printf("read.c : previousfile : after close\n");
        ifile = prev;
        fprintf(stderr, " ]");

        //printf("read.c : previousfile : end\n");
}

//void pushfile(e)
void pushfile()
{
        Ifile *nfile;
        char *n;

        //printf("read.c : pushfile : start\n");
        advanceeol();
        if(istoken()) n = newstring(tokenptr, tokensize());
        else {
                error("(%s:%d) Expected a file name.", ifile->name, ifile->line);
                n = newstring("", 0);
        }
        flushtoken();
        expectnewline();

        nfile = newifile();
        nfile->prev = ifile;
        nfile->name = n;
        nfile->line = 1;
        if((nfile->fd = fopen(n, "r")) == NULL)
                fatal("(%s:%d) Can't open data file %s\n", ifile->name, ifile->line, n);
        ifile = nfile;
        fprintf(stderr, " [ %s", n);
        gobbleblanklines();
        //printf("read.c : pushfile : end\n");
}

