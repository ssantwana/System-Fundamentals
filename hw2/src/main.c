
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "version.h"
#include "global.h"
#include "gradedb.h"
#include "stats.h"
#include "read.h"
#include "write.h"
#include "normal.h"
#include "sort.h"
//#include "error.h"


/*
 * Course grade computation program
 */
/*
#define REPORT          0
#define COLLATE         1
#define FREQUENCIES     2
#define QUANTILES       3
#define SUMMARIES       4
#define MOMENTS         5
#define COMPOSITES      6
#define INDIVIDUALS     7
#define HISTOGRAMS      8
#define TABSEP          9
#define ALLOUTPUT      10
#define SORTBY         11
#define NONAMES        12

static struct option_info {
        unsigned int val;
        char *name;
        char chr;
        int has_arg;
        char *argname;
        char *descr;
} option_table[] = {
 {REPORT,         "report",    'r',      no_argument, NULL,
                  "Process input data and produce specified reports."},
 {COLLATE,        "collate",   'c',      no_argument, NULL,
                  "Collate input data and dump to standard output."},
 {FREQUENCIES,    "freqs",     0,        no_argument, NULL,
                  "Print frequency tables."},
 {QUANTILES,      "quants",    0,        no_argument, NULL,
                  "Print quantile information."},
 {SUMMARIES,      "summaries", 0,        no_argument, NULL,
                  "Print quantile summaries."},
 {MOMENTS,        "stats",     0,        no_argument, NULL,
                  "Print means and standard deviations."},
 {COMPOSITES,     "comps",     0,        no_argument, NULL,
                  "Print students' composite scores."},
 {INDIVIDUALS,    "indivs",    0,        no_argument, NULL,
                  "Print students' individual scores."},
 {HISTOGRAMS,     "histos",    0,        no_argument, NULL,
                  "Print histograms of assignment scores."},
 {TABSEP,         "tabsep",    0,        no_argument, NULL,
                  "Print tab-separated table of student scores."},
 {ALLOUTPUT,      "all",       'a',      no_argument, NULL,
                  "Print all reports."},
 {NONAMES,        "nonames",   'n',      no_argument, NULL,
                  "Suppress printing of students' names."},
 {SORTBY,         "sortby",    'k',      required_argument, "key",
                  "Sort by {name, id, score}."}
};

#define NUM_OPTIONS (13)

static char *short_options = "";
static struct option long_options[NUM_OPTIONS];

static void init_options() {
    for(unsigned int i = 0; i < NUM_OPTIONS; i++) {
        struct option_info *oip = &option_table[i];
        struct option *op = &long_options[i];
        op->name = oip->name;
        op->has_arg = oip->has_arg;
        op->flag = NULL;
        op->val = oip->val;
    }
}

static int report, collate, freqs, quantiles, summaries, moments,
           scores, composite, histograms, tabsep, nonames;
*/
static void usage();


void free_freq(Freqs *freq){
    //    float score;                    /* The raw score */
    //    int count;                      /* Frequency for this score */
    //    int numless;                    /* Number of scores < this */
    //    int numlesseq;                  /* Number of scores <= this */
    //    struct Freqs *next;             /* Pointer to next higher score */
    if(freq->next != NULL){
        free_freq(freq->next);
        free(freq->next);
    }
}

void free_sstats(Sectionstats *sstats){

    //    Assignment *asgt;               /* Assignment stats are for */
    //    Section *section;               /* Section stats are for */
    //    int valid;                      /* Number of valid scores */
    //    int tallied;                    /* Number of scores tallied */
    //    double sum;                     /* Sum of valid scores */
    //    double sumsq;                   /* Sum of squares of valid scores */
    //    float min;                      /* Minimum valid score */
    //    float max;                      /* Maximum valid score */
    //    float mean;                     /* Sample mean for valid scores */
    //    float stddev;                   /* Sample standard deviation */
    //    Freqs *freqs;                   /* Frequency information */
    if(sstats->freqs != NULL){
        free_freq(sstats->freqs);
        free(sstats->freqs);
    }


    //    struct Sectionstats *next;      /* Pointer to data for next section */
    if(sstats->next != NULL){
        free_sstats(sstats->next);
        free(sstats->next);
    }

}

void free_cstats( Classstats *cstats){
    //  Assignment *asgt;               /* The assignment stats are for */
    //    int valid;                      /* Number of valid scores */
    //    int tallied;                    /* Number of scores tallied */
    //    double sum;                     /* Sum of valid scores */
    //    double sumsq;                   /* Sum of squares of valid scores */
    //    float min;                      /* Minimum valid score */
    //    float max;                      /* Maximum valid score */
    //    float mean;                     /* Sample mean for valid scores */
    //    float stddev;                   /* Sample std deviation */

    //    Freqs *freqs;                   /* Frequency information */
    if(cstats->freqs != NULL){
        free_freq(cstats->freqs);
        free(cstats->freqs);
    }

    //   Sectionstats *sstats;           /* Per-section statistics */
    if(cstats->sstats != NULL){
        free_sstats(cstats->sstats);
        free(cstats->sstats);
    }

    //    struct Classstats *next;        /* Pointer to data for next asgt */
    if(cstats -> next != NULL){
        free_cstats(cstats->next);
        free(cstats -> next);
    }
}


void my_free_statics(Stats *stats){

    if(stats == NULL)
        return;

    if(stats->cstats != NULL){
        free_cstats(stats->cstats);
        free(stats->cstats);
    }

    free(stats);

}

void free_scores( Score *score){
    //    struct Assignment *asgt;        /* Assignment grade is for */
    //    float grade;                    /* Numeric grade */
    //    float qnorm;                    /* Substitute quantile score */
    //    float lnorm;                    /* Substitute linear score */
    //    float snorm;                    /* Substitute scale score */
    //    Gflag flag;                     /* Validity flag */
    //    Id code;                        /* Why score is invalid */
    if(score -> code != NULL)
        free(score->code);

    //    Gsubst subst;                   /* Invalid grade substitution info */

    //    struct Classstats *cstats;      /* Pointer to statistics by class */


    //    struct Sectionstats *sstats;    /* Pointer to statistics by section */


    //    struct Score *next;             /* Next score for student */
    if(score->next != NULL){
        free_scores(score->next);
        free(score->next);
    }
}

void free_assignments(Assignment *ass){
    // Id name;                        /* Identifying name of assignment */
    if(ass->name != NULL)
        free(ass->name);

    //    Atype atype;                    /* Assignment type (homework, exam) */
    if(ass->atype != NULL)
        free(ass->atype);

    //    Wpolicy wpolicy;                /* Is assignment weighted? */

    //    float weight;                   /* Weighting of this assignment */

    //    Npolicy npolicy;                /* Normalization to use */
    //    float max;                      /* Maximum possible score */
    //    float mean;                     /* Mean for LINEAR normalization */
    //    float stddev;                   /* Std dev for LINEAR normalization */
    //    float scale;                    /* New maximum for SCALE norm'zation */

    //    Ngroup ngroup;                  /* Group over which to normalize */

    //    struct Assignment *next;        /* Next assignment in course */
    if(ass->next != NULL){
        free_assignments(ass->next);
        free(ass->next);
    }
}

void free_student(Student *student){
    //    Id id;                          /* Student ID number */
    if(student -> id != NULL)
        free(student -> id);

    //    Surname surname;                /* Student's surname */
    if(student -> surname != NULL)
        free(student -> surname);

    //    Name name;                      /* Student's given name */
    if(student -> name != NULL)
        free(student -> name);

    //    Score *rawscores;               /* Student's raw scores */
    if(student -> rawscores != NULL){
        free_scores(student->rawscores);
        free(student-> rawscores);
    }

    //    Score *normscores;              /* Student's normalized scores */
    if(student -> normscores != NULL){
        free_scores(student->normscores);
        free(student-> normscores);
    }

    //    float composite;                /* Student's composite score */
    //    struct Section *section;        /* Pointer to student's section */
    //    struct Student *next;           /* Next student in section roster */

    //    struct Student *cnext;          /* Next student in course roster */
    if(student -> cnext != NULL){
        free_student(student -> cnext);
        free(student->cnext);
    }
}


void free_sections(Section *S){
    //    Name name;                      /* Name or number of section */
    if(S->name != NULL)
        free(S->name);

    //    Assistant *assistant;           /* Assistant in charge of section */
    if(S->assistant != NULL){
        if(S->assistant->name != NULL)
            free(S->assistant->name);
        if(S->assistant->surname != NULL)
            free(S->assistant->surname);
        free(S->assistant);
    }

    //    Student *roster;                /* List of students in section */

    //    struct Section *next;           /* Next section in course */
    if(S->next != NULL){
        free_sections(S->next);
        free(S->next);
    }
}



void my_free(Course *c){
    if(c == NULL) return;
    //Id number;                      /* Course number */
    if(c->number != NULL )
        free(c->number);
    // Name title;                     /* Course title */
    if(c->title != NULL)
        free(c->title);

    // Professor *professor;           /* Professor in charge of course */
    if(c->professor != NULL){
        if(c->professor->surname != NULL)
            free(c->professor->surname);
        if(c->professor->name != NULL)
            free(c->professor->name);
        free(c->professor);
    }

    // Assignment *assignments;        /* List of assignments in course */
    if(c->assignments != NULL){
        free_assignments(c->assignments);
        free(c->assignments);
    }

    // Section *sections;              /* List of sections in course */
    if(c->sections != NULL){
        free_sections(c->sections);
        free(c->sections);
    }

    // Student *roster;
    if(c->roster != NULL){
        free_student(c->roster);
        free(c->roster);
    }

    if(c != NULL)
        free(c);
}
/*
int my_strcmp(char *S1 , char *S2){
    int l1 = strlen(S1);
    int l2 = strlen(S2);
    if(l1 != l2 )
        return 1;
    for(int i = 0; i < l1 ; i++){
        if(S1[i] != S2[i])
            return 1;
    }
    return 0;
}
*/
#define REPORT          0
#define COLLATE         1
#define FREQUENCIES     2
#define QUANTILES       3
#define SUMMARIES       4
#define MOMENTS         5
#define COMPOSITES      6
#define INDIVIDUALS     7
#define HISTOGRAMS      8
#define TABSEP          9
#define ALLOUTPUT      10
#define SORTBY         11
#define NONAMES        12
#define OUTPUT         13

static struct option_info {
        unsigned int val;
        char *name;
        char chr;
        int has_arg;
        char *argname;
        char *descr;
} option_table[] = {
 {REPORT,         "report",    'r',      no_argument, NULL,
                  "Process input data and produce specified reports."},
 {COLLATE,        "collate",   'c',      no_argument, NULL,
                  "Collate input data and dump to standard output."},
 {FREQUENCIES,    "freqs",     0,        no_argument, NULL,
                  "Print frequency tables."},
 {QUANTILES,      "quants",    0,        no_argument, NULL,
                  "Print quantile information."},
 {SUMMARIES,      "summaries", 0,        no_argument, NULL,
                  "Print quantile summaries."},
 {MOMENTS,        "stats",     0,        no_argument, NULL,
                  "Print means and standard deviations."},
 {COMPOSITES,     "comps",     0,        no_argument, NULL,
                  "Print students' composite scores."},
 {INDIVIDUALS,    "indivs",    0,        no_argument, NULL,
                  "Print students' individual scores."},
 {HISTOGRAMS,     "histos",    0,        no_argument, NULL,
                  "Print histograms of assignment scores."},
 {TABSEP,         "tabsep",    0,        no_argument, NULL,
                  "Print tab-separated table of student scores."},
 {ALLOUTPUT,      "all",       'a',      no_argument, NULL,
                  "Print all reports."},
 {NONAMES,        "nonames",   'n',      no_argument, NULL,
                  "Suppress printing of students' names."},
 {SORTBY,         "sortby",    'k',      required_argument, "key",
                  "Sort by {name, id, score}."},
 {OUTPUT,         "output",    'o',      required_argument, "outfile",
                  "Redirect to desired file."}
};

#define NUM_OPTIONS (14)

//static char *short_options = "r:";
static struct option long_options[NUM_OPTIONS];

static void init_options() {
    for(unsigned int i = 0; i < NUM_OPTIONS; i++) {
        struct option_info *oip = &option_table[i];
        struct option *op = &long_options[i];
        op->name = oip->name;
        op->has_arg = oip->has_arg;
        op->flag = NULL;
        op->val = oip->val;
    }
}

static int report, collate, freqs, quantiles, summaries, moments,
           scores, composite, histograms, tabsep, nonames, output;


int main(argc, argv)
int argc;
char *argv[];
{

        Course *c = NULL;
        Stats *s = NULL;
        extern int errors, warnings;
        char optval;
        int (*compare)() = comparename;
        char output_file[50];

        //printf("*** Program started ***\n");

        fprintf(stderr, BANNER);

        //printf("main : trapped\n");

        init_options();
        //printf("main : trapped 2\n");
        //printf("no of arguments : %d\n",argc);
        //printf("optind : %d\n",optind);
        if(argc <= 1){
            //printf("less Arguments\n");
            usage(argv[0]);
            //printf("optind : %d\n",optind);
        }
        while(optind < argc) {
            //printf("main: In while\n");
            if((optval = getopt_long(argc, argv, "arcno:k:" , long_options, NULL)) != -1) {
                //printf("main : in while : before switch : optval :%c\n",optval);
                switch(optval) {

                case 'r' :
                case REPORT: report++;
                    //printf("Position of report : %d\n",optind);
                    if(optind != 2){
                        fprintf(stderr,
                                "Report should be the first argument.\n\n");
                        usage(argv[0]);
                    };
                    break;

                case COLLATE: collate++;
                case 'c': collate++;
                    if(optind != 2){
                        fprintf(stderr,
                                "Collate should be the first argument.\n\n");
                        usage(argv[0]);
                    };
                    break;

                case TABSEP: tabsep++; break;

                case 'n':
                case NONAMES: nonames++; break;

                case SORTBY:
                    //printf("In sortby\n");
                    //printf("Key is : %s\n",optarg);
                    //printf("Printed\n");
                    if(!strcmp(optarg, "name")){
                        //printf("First\n");
                        compare = comparename;
                    }
                    else if(!strcmp(optarg, "id")){
                        //printf("Second\n");
                        compare = compareid;
                    }
                    else if(!strcmp(optarg, "score")){
                        //printf("Third\n");
                        compare = comparescore;
                    }
                    else {
                        fprintf(stderr,
                                "Option '%s' requires argument from {name, id, score}.\n\n",
                                option_table[(int)optval].name);
                        usage(argv[0]);
                    }
                    break;
                case 'k':
                    //printf("In sortby\n");
                    if(!strcmp(optarg, "name"))
                        compare = comparename;
                    else if(!strcmp(optarg, "id"))
                        compare = compareid;
                    else if(!strcmp(optarg, "score"))
                        compare = comparescore;
                    else {
                        fprintf(stderr,
                                "Option '-k' requires argument from {name, id, score}.\n\n");
                        usage(argv[0]);
                    }
                    break;
                case FREQUENCIES: freqs++; break;
                case QUANTILES: quantiles++; break;
                case SUMMARIES: summaries++; break;
                case MOMENTS: moments++; break;
                case COMPOSITES: composite++; break;
                case INDIVIDUALS: scores++; break;
                case HISTOGRAMS: histograms++; break;

                case 'a':
                case ALLOUTPUT:
                    freqs++; quantiles++; summaries++; moments++;
                    composite++; scores++; histograms++; tabsep++;
                    break;

                case OUTPUT:
                case 'o':
                    output++;
                    strcpy(output_file,optarg);
                    //printf("Output file is :%s\n",output_file); break;

                case '?':
                    //printf("Not sure\n");
                    usage(argv[0]);
                    break;

                default:
                    //printf("***** Nothing Matched *****\n");
                    break;
                }
            } else {
                //printf("main :  switch : optval is -1\n");
                break;
            }
        }
        //printf("main : switch over : optind : %d\n",optind);
        if(output == 1 ){
            if(freopen(output_file, "w", stdout) == NULL){
                usage(argv[0]);
            }
        }else{ if(output > 1){
            fprintf(stderr, "More than one output request.\n\n");
            usage(argv[0]);
            }
        }

        if(optind == argc) {
                fprintf(stderr, "No input file specified.\n\n");
                usage(argv[0]);
        }
        char *ifile = argv[optind];
        //usage(*argv);
        //printf("main : creating ifile : \n");
        if(report == 0 && collate == 0) {
                //printf("No of report : %d No of collate : %d\n",report,collate);
                //printf("")
                fprintf(stderr, "Either report or collate must be there is required.\n\n");
                //printf("main : befor printing usage\n");
                usage(argv[0]);
                //printf("main : printing usage done\n");
        }

        //printf("Compared report == collate done\n");

        fprintf(stderr, "Reading input data...\n");
        c = readfile(ifile);
        if(errors) {
           printf("%d error%s found, so no computations were performed.\n",
                  errors, errors == 1 ? " was": "s were");
           my_free(c);
           exit(EXIT_FAILURE);
        }

        fprintf(stderr, "Calculating statistics...\n");
        s = statistics(c);
        if(s == NULL) fatal("There is no data from which to generate reports.");
        normalize(c);
        //normalize(c);
        composites(c);
        sortrosters(c, comparename);
        checkfordups(c->roster);
        if(collate) {
                fprintf(stderr, "Dumping collated data...\n");
                writecourse(stdout, c);
                my_free(c);
                my_free_statics(s);
                exit(errors ? EXIT_FAILURE : EXIT_SUCCESS);
        }
        sortrosters(c, compare);

        fprintf(stderr, "Producing reports...\n");
        reportparams(stdout, ifile, c);
        if(moments) reportmoments(stdout, s);
        if(composite) reportcomposites(stdout, c, nonames);
        if(freqs) reportfreqs(stdout, s);
        if(quantiles) reportquantiles(stdout, s);
        if(summaries) reportquantilesummaries(stdout, s);
        if(histograms) reporthistos(stdout, c, s);
        if(scores) reportscores(stdout, c, nonames);
        //if(tabsep) reporttabs(stdout, c, nonames);
        if(tabsep) reporttabs(stdout, c);

        fprintf(stderr, "\nProcessing complete.\n");
        printf("%d warning%s issued.\n", warnings+errors,
               warnings+errors == 1? " was": "s were");
        my_free(c);
        my_free_statics(s);
        exit(errors ? EXIT_FAILURE : EXIT_SUCCESS);
}

void usage(name)
char *name;
{
        //printf("In Usage [^_^]\n");
        struct option_info *opt;

        fprintf(stderr, "Usage: %s [options] <data file>\n", name);
        fprintf(stderr, "Valid options are:\n");
        //debug("test");
        //printf("In usage\n");
        for(unsigned int i = 0; i < NUM_OPTIONS; i++) {
                //printf("usage : i : %d\n",i);
                opt = &option_table[i];
                char optchr[5] = {' ', ' ', ' ', ' ', '\0'};
                if(opt->chr)
                  sprintf(optchr, "-%c, ", opt->chr);
                char arg[32];
                if(opt->has_arg)
                    sprintf(arg, " <%.10s>", opt->argname);
                else
                    sprintf(arg, "%.13s", "");
                fprintf(stderr, "\t%s--%-10s%-13s\t%s\n",
                            optchr, opt->name, arg, opt->descr);
                opt++;
        }
        exit(EXIT_FAILURE);
}
