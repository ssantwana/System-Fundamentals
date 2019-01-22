
/*
 * Allocate storage for the various data structures
 */

char *memerr = "Unable to allocate memory.";

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "global.h"
#include "gradedb.h"
#include "stats.h"
#include "allocate.h"

Professor *newprofessor()
{
        Professor *p;
        if((p = (Professor *)calloc(sizeof(Professor),1)) == NULL)
                fatal(memerr);
        return(p);
}

Assistant *newassistant()
{
        Assistant *a;
        if((a = (Assistant *)calloc(sizeof(Assistant),1)) == NULL)
                fatal(memerr);
        return(a);
}

Student *newstudent()
{
        Student *s;
        if((s = (Student *)calloc(sizeof(Student),1)) == NULL)
                fatal(memerr);
        return(s);
}

Section *newsection()
{
        Section *s;
        if((s = (Section *)calloc(sizeof(Section),1)) == NULL)
                fatal(memerr);
        return(s);
}

Assignment *newassignment()
{
        Assignment *a;
        if((a = (Assignment *)calloc(sizeof(Assignment),1)) == NULL)
                fatal(memerr);
        return(a);
}

Course *newcourse()
{
        Course *c;
        if((c = (Course *)calloc(sizeof(Course),1)) == NULL)
                fatal(memerr);
        return(c);
}

Score *newscore()
{
        Score *s;
        if((s = (Score *)calloc(sizeof(Score),1)) == NULL)
                fatal(memerr);
        return(s);
}

char *newstring(tp, size)
char *tp;
int size;
{
        char *s, *cp;
        if((s = (char *)calloc(size,1)) == NULL)
                fatal(memerr);
        cp = s;
        while(size-- > 0) *cp++ = *tp++;
        return(s);
}

Freqs *newfreqs()
{
        Freqs *f;
        if((f = (Freqs *)calloc(sizeof(Freqs),1)) == NULL)
                fatal(memerr);
        return(f);
}

Classstats *newclassstats()
{
        Classstats *c;
        if((c = (Classstats *)calloc(sizeof(Classstats),1)) == NULL)
                fatal(memerr);
        return(c);

}

Sectionstats *newsectionstats()
{
        Sectionstats *s;
        if((s = (Sectionstats *)calloc(sizeof(Sectionstats),1)) == NULL)
                fatal(memerr);
        return(s);

}

Stats *newstats()
{
        Stats *s;
        if((s = (Stats *)calloc(sizeof(Stats),1)) == NULL)
                fatal(memerr);
        return(s);
}

Ifile *newifile()
{
        Ifile *f;
        if((f = (Ifile *)calloc(sizeof(Ifile),1)) == NULL)
                fatal(memerr);
        return(f);
}
