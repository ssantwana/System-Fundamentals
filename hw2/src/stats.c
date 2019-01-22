
/*
 * Compute Per-assignment Statistics
 */

#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include "global.h"
#include "gradedb.h"
#include "stats.h"
#include "allocate.h"

Course *debug_course = NULL;

Stats *statistics(c)
Course *c;
{
        Stats *s;
        //printf("stats.c : statistics : start\n");
        s = buildstats(c);               /* Build "stats" data structure */
        //test(c);
        //printf("stats.c : statistics : buildstats done\n");
        if(s == NULL) return(s);        /* No data! */
        //printf("stats.c : statistics : before do_link\n");
        do_links(c, s);                 /* Fill in pointers */
        //printf("stats.c : statistics : after do_link\n");
        //test(c);
        //printf("stats.c : statistics : do_link done\n");
        //test(c);
        do_freqs(c);                    /* Count valid scores */
        //test(c);
        //printf("stats.c : statistics : do_freq done\n");
        //test(c);
        do_quantiles(s);                /* Fill in quantile information */
        do_sums(c);                     /* Prepare to compute moments */
        do_moments(s);                  /* Now compute moments */
        //printf("stats.c : statistics : end\n");
        return(s);
}

Stats *buildstats(c)
Course *c;
{
        Stats *stats;
        Classstats *csp;
        Sectionstats *ssp;
        Assignment *ap;
        Section *sp;

        stats = newstats();
        stats->cstats = NULL;
        csp = NULL;
        for(ap = c->assignments; ap != NULL; ap = ap->next) {
                if(csp == NULL) {
                        stats->cstats = newclassstats();
                        csp = stats->cstats;
                } else {
                        csp->next = newclassstats();
                        csp = csp->next;
                }
                csp->next = NULL;
                csp->asgt = ap;
                csp->valid = 0;
                csp->tallied = 0;
                csp->sum = csp->sumsq = 0.0;
                csp->freqs = NULL;
                csp->sstats = NULL;
                ssp = NULL;
                for(sp = c->sections; sp != NULL; sp = sp->next) {
                        if(ssp == NULL) {
                                csp->sstats = newsectionstats();
                                ssp = csp->sstats;

                        } else {
                                ssp->next = newsectionstats();
                                ssp = ssp->next;
                        }
                        ssp->next = NULL;
                        ssp->asgt = ap;
                        ssp->section = sp;
                        ssp->valid = 0;
                        ssp->tallied = 0;
                        ssp->sum = ssp->sumsq = 0.0;
                        ssp->freqs = NULL;
                }
        }
        return(stats);
}

/*
 * Fill in pointers to make it easier to do stuff.
 *
 *      For each assignment in the course,
 *       for each section
 *        for each student in that section,
 *              link students into course roster.
 *         for each score for that student,
 *              link score to class and section statistics.
 */

void do_links(c, s)
Course *c;
Stats *s;
{
        Assignment *ap;
        Section *sep;
        Student *stp, *rp;
        Score *scp;

        Classstats *csp;
        Sectionstats *ssp;

        csp = s->cstats;
        c->roster = rp = NULL;
        for(ap = c->assignments; ap != NULL; ap = ap->next, csp = csp->next) {
           ssp = csp->sstats;
           for(sep = c->sections; sep != NULL; sep = sep->next, ssp = ssp->next) {
              for(stp = sep->roster; stp != NULL; stp = stp->next) {
                 if(rp == NULL) {       /* Link students into course roster */
                        c->roster = stp;
                        stp->cnext = NULL;
                        rp = stp;
                 } else {
                        rp->cnext = stp;
                        stp->cnext = NULL;
                        rp = stp;
                 }
                 for(scp = stp->rawscores; scp != NULL; scp = scp->next) {
                    if(scp->asgt == ap) {
                       scp->cstats = csp; /* link scores to statistics for */
                       scp->sstats = ssp; /* later use in normalization */
                    }
                 }
              }
           }
        }
        //printf("stats.c : do_link : c->p : %p\n" , (void *)c->professor);
}

/*
 * Construct frequency tables:
 *      For each student in the course roster,
 *        for each score for that student,
 *          if that score is a valid one, or if it has USERAW substitution,
 *               insert it into the class frequency tables for the
 *               appropriate assignment, and also into the section
 *               frequency tables for the appropriate assignment and section.
 */

void do_freqs(c)
Course *c;
{
        Student *stp;
        Score *scp;
        debug_course = c;
        //printf("stats.c : do_freq : professor ptr : %p\n",(void *)c->professor);
        //printf("stats.c : do_freq : start\n");
        for(stp = c->roster; stp != NULL; stp = stp->cnext) {
              //printf("stats.c :         do_freq : professor ptr : %p\n",(void *)c->professor);
           for(scp = stp->rawscores; scp != NULL; scp = scp->next) {
              //printf("stats.c :     do_freq : professor ptr : %p\n",(void *)c->professor);
               if(scp->flag == VALID || scp->subst == USERAW) {
                  //printf("stats.c : do_freq : professor ptr : %p\n",(void *)c->professor);
                /*if(scp->cstats == NULL){
                  printf("\n\n\n\t\t\t*****\n\n\n");
                }*/
                  scp->cstats->freqs = count_score(scp, scp->cstats->freqs);
                  //printf("stats.c : do_freq : professor ptr : %p  and scp->cstats->freqs  : %p Got it\n",(void *)c->professor,(void *)scp->cstats->freqs);
                  scp->cstats->tallied++;
                  //printf("stats.c : do_freq : professor ptr : %p\n",(void *)c->professor);
                  scp->sstats->freqs = count_score(scp, scp->sstats->freqs);
                  //printf("stats.c : do_freq : professor ptr : %p\n",(void *)c->professor);
                  scp->sstats->tallied++;
                  //printf("stats.c : do_freq : professor ptr : %p\n",(void *)c->professor);
               }
           }
        }

        //printf("stats.c : do_freq : professor ptr : %p\n",(void *)c->professor);
        //printf("stats.c : do_freq : end\n");
}

/*
 * Count the given score either by incrementing a count in an existing
 * bucket, or else inserting a new bucket with a count of one.
 * The head of the new list is returned.
 */

Freqs *count_score(scp, afp)
Score *scp;
Freqs *afp;
{
        Freqs *fp = NULL, *sfp = NULL;

        //printf("stats.c : count_score : professor ptr : start %p\n",(void *)debug_course->professor);

        for(fp = afp; fp != NULL; sfp = fp, fp = fp->next) {
                if(fp->score == scp->grade) {
                        fp->count++;
                        return(afp);
                } else if(fp->score > scp->grade) {
                        if(sfp == NULL) {       /* insertion at head */
                                sfp = newfreqs();
                                sfp->next = fp;
                                sfp->score = scp->grade;
                                sfp->count = 1;
                                return(sfp);    /* return new head */
                        } else {                /* insertion in middle */
                                sfp->next = newfreqs();
                                sfp = sfp->next;
                                sfp->next = fp;
                                sfp->score = scp->grade;
                                sfp->count = 1;
                                return(afp);    /* return old head */
                        }
                } else continue;
        }

        //printf("stats.c : count_score : professor ptr : After for %p\n",(void *)debug_course->professor);
        if( sfp == NULL) {       /* insertion into empty list */
                sfp = newfreqs();
                sfp->next = NULL;
                sfp->score = scp->grade;
                sfp->count = 1;
                //printf("stats.c : count_score : professor ptr : in If %p\n",(void *)debug_course->professor);
                return(sfp);
                //return NULL;    /* return new head */
        } else {                /* insertion at end of list */

                //printf("stats.c : count_score : professor ptr : in Else : start %p and sfp->next%p\n",(void *)debug_course->professor,(void *)sfp->next);
                //void *test_ptr = (void *)newfreqs();
                //printf("test ptr : %p and curr sfp->next : %p \n",test_ptr,(void *)sfp->next);
                sfp->next = newfreqs();
                //sfp->next = test_ptr;
                //printf("stats.c : count_score : professor ptr : in Else : after new freq %p\n",(void *)debug_course->professor);
                sfp = sfp->next;
                sfp->next = NULL;
                //printf("stats.c : count_score : professor ptr : in Else : after updating sfp %p\n",(void *)debug_course->professor);
                sfp->score = scp->grade;
                sfp->count = 1;
                //printf("stats.c : count_score : professor ptr : in Else  end%p\n",(void *)debug_course->professor);

                return(afp);
        }
}

/*
 * Traverse all the frequency tables and sum up counts to produce
 * quantile information.  This saves time in score normalization.
 */

void do_quantiles(s)
Stats *s;
{
        Classstats *csp;
        Sectionstats *ssp;
        Freqs *fp;
        int sum;

        for(csp = s->cstats; csp != NULL; csp = csp->next) {
           sum = 0;
           for(fp = csp->freqs; fp != NULL; fp = fp->next) {
              fp->numless = sum;
              sum += fp->count;
              fp->numlesseq = sum;
           }
           for(ssp = csp->sstats; ssp != NULL; ssp = ssp->next) {
              sum = 0;
              for(fp = ssp->freqs; fp != NULL; fp = fp->next) {
                fp->numless = sum;
                sum += fp->count;
                fp->numlesseq = sum;
              }
           }
        }
}

/*
 * Compute sums necssary for determining moments:
 *      For each student in the course,
 *       for each score for that student,
 *            if that score is a valid one for the considered assigment,
 *              or if it has USERAW substitution,
 *            incorporate it into the sums for the whole class and
 *            for the section.
 */

void do_sums(c)
Course *c;
{
        Student *stp;
        Score *scp;
        Classstats *csp;
        Sectionstats *ssp;
        double g;

        for(stp = c->roster; stp != NULL; stp = stp->cnext) {
           for(scp = stp->rawscores; scp != NULL; scp = scp->next) {
              if(scp->flag == VALID || scp->subst == USERAW) {
                csp = scp->cstats;
                ssp = scp->sstats;
                g = scp->grade;
                if(csp->valid++ == 0) csp->min = csp->max = g;
                else {
                        if(g < csp->min) csp->min = g;
                        if(g > csp->max) csp->max = g;
                }
                if(ssp->valid++ == 0) ssp->min = ssp->max = g;
                else {
                        if(g < csp->min) csp->min = g;
                        if(g > csp->max) csp->max = g;
                }
                csp->sum += g;
                ssp->sum += g;
                g = g*g;
                csp->sumsq += g;
                ssp->sumsq += g;
              }
           }
        }
}

/*
 * Traverse the data structure and use the accumulated data
 * to fill in the moments and quantiles.
 */

void do_moments(sp)
Stats *sp;
{
        Classstats *csp;
        Sectionstats *ssp;

        for(csp = sp->cstats; csp != NULL; csp = csp->next) {
           if(csp->valid) {
                csp->mean = csp->sum/csp->valid;
                if(csp->valid == 1) {
                   warning("Too few scores for %s.", csp->asgt->name);
                   csp->stddev = 0.0;
                } else {
                   csp->stddev = stddev(csp->valid, csp->sum, csp->sumsq);
                }
           } else {
                warning("No valid scores for %s.", csp->asgt->name);
                csp->mean = 0.0;
                csp->stddev = 0.0;
           }
           for(ssp = csp->sstats; ssp != NULL; ssp = ssp->next) {
              if(ssp->valid) {
                 ssp->mean = ssp->sum/ssp->valid;
                 if(ssp->valid == 1) {
                        warning("Too few scores for %s, section %s.",
                                ssp->asgt->name, ssp->section->name);
                        ssp->stddev = 0.0;
                 } else {
                    ssp->stddev = stddev(ssp->valid, ssp->sum, ssp->sumsq);
                 }
              } else {
                 warning("No valid scores for %s, section %s.",
                         ssp->asgt->name, ssp->section->name);
                 ssp->mean = 0.0;
                 ssp->stddev = 0.0;
              }
           }
        }
}

double stddev(n, sum, sumsq)
int n;
double sum;
double sumsq;
{
        if(n >= 2) return(sqrt((sumsq - (sum*sum)/n)/(n-1)));
        else return(0.0);
}
