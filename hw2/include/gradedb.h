// Personal



//From gradedb.h

//#include "debug.h"

typedef char *Surname;                  /* Person's surname */
typedef char *Name;                     /* Course name or person's name */
typedef char *Id;                       /* Short identifying name or number */
typedef char *Atype;                    /* Assignment type */

typedef enum { NOWEIGHT, WEIGHT } Wpolicy;
typedef enum { RAW, LINEAR, QUANTILE, SCALE } Npolicy;
typedef enum { BYCLASS, BYSECTION } Ngroup;

typedef enum { VALID, INVALID } Gflag;
typedef enum { USERAW, USENORM, USELIKEAVG, USECLASSAVG } Gsubst;


typedef struct Score {
        struct Assignment *asgt;        /* Assignment grade is for */
        float grade;                    /* Numeric grade */
        float qnorm;                    /* Substitute quantile score */
        float lnorm;                    /* Substitute linear score */
        float snorm;                    /* Substitute scale score */
        Gflag flag;                     /* Validity flag */
        Id code;                        /* Why score is invalid */
        Gsubst subst;                   /* Invalid grade substitution info */
        struct Classstats *cstats;      /* Pointer to statistics by class */
        struct Sectionstats *sstats;    /* Pointer to statistics by section */
        struct Score *next;             /* Next score for student */
} Score;

typedef struct Student {
        Id id;                          /* Student ID number */
        Surname surname;                /* Student's surname */
        Name name;                      /* Student's given name */
        Score *rawscores;               /* Student's raw scores */
        Score *normscores;              /* Student's normalized scores */
        float composite;                /* Student's composite score */
        struct Section *section;        /* Pointer to student's section */
        struct Student *next;           /* Next student in section roster */
        struct Student *cnext;          /* Next student in course roster */
} Student;

typedef struct {
        Surname surname;                /* Professor's surname */
        Name name;                      /* Professor's given name */
} Professor;

typedef struct {
        Surname surname;                /* Assistant's surname */
        Name name;                      /* Assistant's given name */
} Assistant;



typedef struct Section {
        Name name;                      /* Name or number of section */
        Assistant *assistant;           /* Assistant in charge of section */
        Student *roster;                /* List of students in section */
        struct Section *next;           /* Next section in course */
} Section;




typedef struct Assignment {
        Id name;                        /* Identifying name of assignment */
        Atype atype;                    /* Assignment type (homework, exam) */
        Wpolicy wpolicy;                /* Is assignment weighted? */
        float weight;                   /* Weighting of this assignment */
        Npolicy npolicy;                /* Normalization to use */
        float max;                      /* Maximum possible score */
        float mean;                     /* Mean for LINEAR normalization */
        float stddev;                   /* Std dev for LINEAR normalization */
        float scale;                    /* New maximum for SCALE norm'zation */
        Ngroup ngroup;                  /* Group over which to normalize */
        struct Assignment *next;        /* Next assignment in course */
} Assignment;










/*
 * Data structures to hold per-assignment statistics
 */

/*
 * Frequency information is stored as a linked list of "Freqs" buckets,
 * sorted from lowest raw score to highest raw score.  Quantiles can
 * be computed by traversing the list looking for the score of interest,
 * and then using the frequency information in that bucket.
 */

typedef struct Freqs {
        float score;                    /* The raw score */
        int count;                      /* Frequency for this score */
        int numless;                    /* Number of scores < this */
        int numlesseq;                  /* Number of scores <= this */
        struct Freqs *next;             /* Pointer to next higher score */
} Freqs;

/*
 * The statistical data for assignments are accumulated in the
 * "Classstats" and "Sectionstats' data structures.
 * The "Classstats" structure contains data on one assignment
 * for the whole class, and a pointer to a list of "Sectionstats"
 * structures, which have data on that same assignment, only
 * for each section separately.
 */

typedef struct Sectionstats {
        Assignment *asgt;               /* Assignment stats are for */
        Section *section;               /* Section stats are for */
        int valid;                      /* Number of valid scores */
        int tallied;                    /* Number of scores tallied */
        double sum;                     /* Sum of valid scores */
        double sumsq;                   /* Sum of squares of valid scores */
        float min;                      /* Minimum valid score */
        float max;                      /* Maximum valid score */
        float mean;                     /* Sample mean for valid scores */
        float stddev;                   /* Sample standard deviation */
        Freqs *freqs;                   /* Frequency information */
        struct Sectionstats *next;      /* Pointer to data for next section */
} Sectionstats;

typedef struct Classstats {
        Assignment *asgt;               /* The assignment stats are for */
        int valid;                      /* Number of valid scores */
        int tallied;                    /* Number of scores tallied */
        double sum;                     /* Sum of valid scores */
        double sumsq;                   /* Sum of squares of valid scores */
        float min;                      /* Minimum valid score */
        float max;                      /* Maximum valid score */
        float mean;                     /* Sample mean for valid scores */
        float stddev;                   /* Sample std deviation */
        Freqs *freqs;                   /* Frequency information */
        Sectionstats *sstats;           /* Per-section statistics */
        struct Classstats *next;        /* Pointer to data for next asgt */
} Classstats;

/*
 * The data for a whole class is headed by a "Stats" structure
 */




















/*
 * Data structures stored in grades database
 */

//void warning(char *fmt);//, char *a1, char *a2, char *a3, char *a4, char *a5, char *a6);
//void warning(char *fmt, char *a1,char *a2,char * a3,char * a4,char * a5,char * a6);
#include <stdio.h>
//#include "stats.h"
void warning(char *fmt, ...);
void error(char *fmt, ...);
void fatal(char *fmt, ...);
int checktoken(char *key);
int istoken();
int tokensize();
//void histo(FILE *fd, int *bins, float , float , int cnt);



typedef struct {
        Id number;                      /* Course number */
        Name title;                     /* Course title */
        Professor *professor;           /* Professor in charge of course */
        Assignment *assignments;        /* List of assignments in course */
        Section *sections;              /* List of sections in course */
        Student *roster;                /* List of students in course */
} Course;


typedef struct Stats {
        Classstats *cstats;             /* List of per-assignment data */
} Stats;

void reportparams(FILE *fd, char *fn,Course *c);
void reportfreqs(FILE *fd,Stats *s);
void reportquantilesummaries(FILE *fd,Stats *s);
void reportquantiles(FILE *fd,Stats *s);
void reportmoments(FILE *fd,Stats *s);
void reportscores(FILE *fd,Course *s , int nm);
void reportcomposites(FILE *fd,Course *s , int nm);
void reporthistos(FILE *fd,Course *c,Stats *s);
void reporttabs(FILE *fd, Course *c);





//void test(Course *c);
//