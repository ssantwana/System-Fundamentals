#include <stdio.h>
#include <criterion/criterion.h>
#include "gradedb.h"
#include "read.h"
#include "write.h"
#include "sort.h"
#include "stats.h"
#include "normal.h"

#define TEST_FILE "cse307.dat"
#define COLLATED_REF "rsrc/cse307.collated"
#define TABSEP_REF "rsrc/cse307.tabsep"
#define COLLATED_OUTPUT "cse307.collated"
#define TABSEP_OUTPUT "cse307.tabsep"

extern int errors, warnings;


Test(basic_suite, read_file_test) {
    Course *c;
    c = readfile(TEST_FILE);
    cr_assert_eq(errors, 0, "There were errors reported when reading test data.\n");
    cr_assert_neq(c, NULL, "NULL pointer returned from readfile().\n");
}


Test(basic_suite, stats_test) {
    Course *c;
    Stats *s;
    c = readfile(TEST_FILE);
    cr_assert_eq(errors, 0, "There were errors reported when reading test data.\n");
    cr_assert_neq(c, NULL, "NULL pointer returned from readfile().\n");
    s = statistics(c);
    cr_assert_neq(s, NULL, "NULL pointer returned from statistics().\n");
}




Test(basic_suite, collate_test) {
    Course *c;
    //Professor *p;
    c = readfile(TEST_FILE);
    //printf("grades_tests.c : test3 :reading file done\n");
    //test(c);
    cr_assert_eq(errors, 0, "There were errors reported when reading test data.\n");
    //printf("grades_tests.c : test3 : asserting error done\n");
    cr_assert_neq(c, NULL, "NULL pointer returned from readfile().\n");
    //printf("grades_tests.c : test3 :before opening file\n");
    FILE *f = fopen(COLLATED_OUTPUT, "w");
    //printf("grades_tests.c : test3 :after opening file\n");
    cr_assert_neq(f, NULL, "Error opening test output file.\n");
    //test(c);
    //printf("grades_tests.c : test3 :before statistics\n");
    //test(c);
    statistics(c);
    //printf("grades_tests.c : test3 :after statistics\n");
    //test(c);
    sortrosters(c, comparename);
    //printf("grades_tests.c : test3 : after sortroster\n");
    writecourse(f, c);
    //printf("grades_tests.c : test3 : write course\n");
    fclose(f);
    //printf("grades_tests.c : test3 :file closed\n");
    char cmd[100];
    sprintf(cmd, "cmp %s %s", COLLATED_OUTPUT, COLLATED_REF);
    int err = system(cmd);
    //printf("Error is %d\n",err);
    cr_assert_eq(err, 0, "Output file doesn't match reference output.\n");
}


Test(basic_suite, tabsep_test) {
    Course *c;
    Stats *s;
    c = readfile(TEST_FILE);
    cr_assert_eq(errors, 0, "There were errors reported when reading test data.\n");
    cr_assert_neq(c, NULL, "NULL pointer returned from readfile().\n");
    //printf("Reg Test :  file reading done\n");
    s = statistics(c);
    cr_assert_neq(s, NULL, "NULL pointer returned from statistics().\n");
    //printf("Reg Test :  fstatistics over\n");
    normalize(c);
    //printf("Reg Test :  normalize over\n");
    composites(c);
    //printf("Reg Test :  composite over over\n");
    sortrosters(c, comparename);
    //printf("Reg Test :  sortroster over\n");
    FILE *f = fopen(TABSEP_OUTPUT, "w");
    cr_assert_neq(f, NULL, "Error opening test output file.\n");
    reporttabs(f, c);
    fclose(f);
    char cmd[100];
    sprintf(cmd, "cmp %s %s", TABSEP_OUTPUT, TABSEP_REF);
    int err = system(cmd);

    //printf("Error is %d\n",err);
    cr_assert_eq(err, 0, "Output file doesn't match reference output.\n");
}

//############################################
// STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
// DO NOT DELETE THESE COMMENTS
//############################################

