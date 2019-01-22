

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//#include "debug.h"
//#include <string.h>
#include "my_lib.h"
//#include "implementation.h"
//#include "memory.h"

#include "imprimer.h"

int input_over = 0;

int declaring_type(char *line);
void print_help_msg();
void declaring_printer(char *line);
void declaring_conversion(char *line);
void get_printers();
void get_jobs();
void print_request(char *line);
void cancel_request(char *line);
void pause_request(char *line);
void resume_request(char *line);
void disable_request(char *line);
void enable_request(char *line);
char *get_line(char *line_buffer,int in);
int scan_line(char *buffer);
/*
 * "Imprimer" printer spooler.
 */

void test(){
    printf("Check fo all nos\n");
}

int main(int argc, char *argv[])
{
    char optval;

    char *file_name = NULL;
    int flag_input = 0, flag_output = 0;
    signal(SIGCHLD, handler);
    int in = 0 ;
    int out  = 1;

    initialize_printer_array(printer_array);
    int fd =-1;

    while(optind < argc) {
        debug("optind : %d and argc : %d",optind,argc);
	if((optval = getopt(argc, argv, "i:o:")) != -1) {
        debug("optval : %c" , optval);
	    switch(optval) {
            case 'i':
                debug("Expecting input file ");
                if(flag_input == 1){
                    fprintf(stderr, "Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
                    break;
                    //exit(EXIT_FAILURE);
                }
                flag_input = 1;
                file_name = optarg;
                debug("Input file is : %s",file_name);
                fd = open(file_name,O_RDONLY);
                if(fd < 0){
                    my_error("Error : file opening error\n");
                    return 1;
                }
                dup2(fd,0);
                in = fd;
                //in = fd;
                // To do dup for input file
                break;
            case 'o':
                debug("Expecting output file ");
                if(flag_output == 1){
                    fprintf(stderr, "Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                flag_output = 1;
                file_name = optarg;
                debug("Output file is : %s",file_name);
                fd = open(file_name,O_WRONLY|O_CREAT);
                if(fd < 0){
                    debug("Fail fd is : %d",fd);
                    my_error("Error : file opening error\n");
                    return 1;;
                }
                dup2(fd,1);
                out = fd;
                // To do dup for output file
                break;

	        case '?':
                my_error("Wrong command : ");
                //fprintf(stderr, "Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
                //exit(EXIT_FAILURE);
                break;

            default:
                my_error("Wrong command : ");
                //printf(stderr, "Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
                //exit(EXIT_FAILURE);
                break;
	    }
	}else{
        //my_error("Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    //char *prefix = "imp>" ;
    char line_memory[1000];
    char *line_buffer = line_memory;
    //char *line = NULL;
    line_buffer = get_line(line_memory,in);
    while(line_buffer == NULL){
        line_buffer = get_line(line_memory,in);
    }
    while( strcmp(line_buffer,"exit") != 0 ){
        if(strcmp(line_buffer,"help") == 0)
            print_help_msg();
        else if(memcmp(line_buffer,"type ",5) == 0){
            declaring_type(line_buffer);
        }
        else if(memcmp(line_buffer,"printer ",8) == 0)
            declaring_printer(line_buffer);
        else if(memcmp(line_buffer,"conversion ",11) == 0)
            declaring_conversion(line_buffer);
        else if(strcmp(line_buffer,"printers") == 0)
            get_printers();
        else if(strcmp(line_buffer,"jobs") == 0)
            get_jobs();
        else if(memcmp(line_buffer,"print ",6) == 0)
            print_request(line_buffer);
        else if(memcmp(line_buffer,"cancel ",7) == 0)
            cancel_request(line_buffer);
        else if(memcmp(line_buffer,"pause ",6) == 0)
            pause_request(line_buffer);
        else if(memcmp(line_buffer,"resume ",7) == 0)
            resume_request(line_buffer);
        else if(memcmp(line_buffer,"disable ",8) == 0)
            disable_request(line_buffer);
        else if(memcmp(line_buffer,"enable ",7) == 0)
            enable_request(line_buffer);
        else{
            // To Do check and implement what if the command is not matched
            debug("Command not recognized");
        }
        if(input_over == 1)
            break;
        line_buffer = get_line(line_memory,in);
        while(line_buffer == NULL){
            if(input_over == 1)
                break;
            line_buffer = get_line(line_memory,in);
        }
        if(input_over == 1)
            break;
    }
    free_all();

    debug("Main Finished!");
    if(in != 0){
        close(in);
    }
    if(out != 1){
        close(out);
    }
    //testing_header();

    exit(EXIT_SUCCESS);
}

void print_help_msg(){
    char *name = "Miscellaneous commands\nhelp\nquit\nConfiguration commands\ntype file_type\nprinter printer_name file_type\nconversion file_type1 file_type2 conversion_program [arg1 arg2 ...]\nInformational commands\nprinters\njobs\nSpooling commands\nprint file_name [printer1 printer2 ...]\ncancel job_number\npause job_number\nresume job_number\ndisable printer_name\nenable printer_name";
    printf("This is help msg : %s\n",name);
}

int declaring_type(char *line){
    debug("Must be declared : recievied : %s",line);
    line = line + 5;
    line = my_trim(line);
    if( line  == NULL){
        my_error("Invalid type");
        return 0;
    }
    int len = strlen(line);
    debug("New Length is : %d",len);
    char *array_of_substrings[len];
    int segments = update_array_of_substring(line,array_of_substrings);
    if(segments != 1){
        my_error("Error : unexpected no of arguments");
        return 0;
    }
    if(type_exist(line)){
        my_error(" Error : type_already_exist");
        return 0;
    }
    add_type_to_list(line);
    check();
    return 1;
}


void declaring_printer(char *line){
    line  = line + 8;
    debug("Line before trim : %s",line);
    line = my_trim(line);
    if(line == NULL){
        my_error("Error : unexpected no of arguments");
        return;
    }

    //debug("Line after trim : %s",line);
    int len = strlen(line);
    //debug("New Length is : %d",len);
    char *array_of_substrings[len];
    int segments = update_array_of_substring(line,array_of_substrings);
    if(segments != 2){
        my_error("Error : unexpected no of arguments :");
        return;
    }
    if(no_of_printers >= 32){
        my_error("Error : No more printers needed");
        return;
    }
    if(printer_exist(array_of_substrings[0])){
        my_error("Error : Printer already exist");
        return;
    }
    if(!type_exist(array_of_substrings[1])){
        my_error("Error : type not recognized");
        return;
    }
    debug("About to add printer : %s with type : %s",array_of_substrings[0],array_of_substrings[1]);
    PRINTER *printer = add_printer(array_of_substrings[0],array_of_substrings[1]);
    my_printer(printer);
    //debug("To implement printer declaration");
}

void declaring_conversion(char *line){
    line = line + 11;
    line = my_trim(line);
    int len = strlen(line);
    debug("New Length is : %d",len);
    char *array_of_substrings[len];
    int segments = update_array_of_substring(line,array_of_substrings);
    if(segments < 3){
        my_error("My_Error : unexpected no of arguments");
        return;
    }
    if( !type_exist(array_of_substrings[0])  || !type_exist( array_of_substrings[1]) ){
        my_error("Error : Unrecognized type\n");
        return;
    }
    if(add_converter(array_of_substrings[0],array_of_substrings[1],array_of_substrings[2] ,array_of_substrings , segments ) == 0)
        return;
    debug("Conversion finished without errorsfor line : %s",line);
}

void get_printers(){
    for(int i = 0 ; i < no_of_printers ; i++){
        char *temp = imp_format_printer_status(printer_array[i], printer_buffer, 200);
        printf("%s\n",temp);

    }
    debug("To implement get printers");
}

void get_jobs(){
    debug("To implement get jobs");
    JOB *job = jobs_head;
    while( job != NULL){
        if(job->status != QUEUED){
            job = job->other_info;
            continue;
        }
        char *temp = imp_format_job_status( job , job_buffer ,200  );
        printf("%s\n",temp);
        job = job->other_info;
    }
}

void print_request(char *line){
    line += 5;
    line = my_trim(line);
    if(line == NULL){
        my_error("Unexpected no of arguments");
        return;
    }
    int len = strlen(line);
    debug("New Length is : %d",len);
    char *array_of_substrings[len];
    int segments = update_array_of_substring(line,array_of_substrings);
    //char *printer = array_of_substrings[0];
    debug("No of segments:%d",segments);
    char *file = array_of_substrings[0];
    char *file_type = get_file_type_from_file(file);
    debug("File :%s== File type:%s",file,file_type);
    if( file_type  == NULL || !type_exist(file_type) ){
        my_error("Error : bad_file_typ ");
        return;
    }
    PRINTER_SET set = ANY_PRINTER;
    if(segments > 1){
        set = get_printer_set(segments - 1 , array_of_substrings + 1);
        if(set  == 0){
            my_error("Error : Issue in printer set\n");
            return;
        }
    }
    JOB *new_job = add_job(file,set,file_type);
    debug("Added job\n");
    my_job(new_job);
    //check();
    debug("Printer request sucessfully added and returned ");


    // To Do implement file type checking




    debug("To implement  print request : %s",line);
}

void cancel_request(char *line){
    line  = line + 6;
    debug("Line: %s",line);
    line = my_trim(line);
    if(line == NULL){
        my_error("Error : unexpected no of arguments");
        return;
    }
    int len = strlen(line);
    debug("New Length is : %d",len);
    char *array_of_substrings[len];
    int segments = update_array_of_substring(line,array_of_substrings);
    if(segments != 1){
        my_error("Error : more arguments than expected");
        return;
    }
    int id = atoi(array_of_substrings[0]);
    if( id < 0 || id >= job_no){
        my_error("Invalid job id");
        return;
    }
    JOB *job = get_job_for_jobid(id);
    if(job == NULL){
        my_error("Invalid job id");
        return;
    }
    if( job->status   != RUNNING ){
        remove_from_jobs_queue(id);
    }
    int pid = job->pgid;
    int result = killpg(pid,SIGTERM);
    if(result < 0){
        my_error("Not able to cancel");
    }
    debug("SIG KILL FINISHED : %d",result);
    return;
    debug("To implement  cancel request : %s",line);
}

void pause_request(char *line){
    line  = line + 6;
    line = my_trim(line);
    if(line == NULL){
        my_error("Error : unexpected no of arguments");
        return;
    }
    int len = strlen(line);
    debug("New Length is : %d",len);
    char *array_of_substrings[len];
    int segments = update_array_of_substring(line,array_of_substrings);
    if(segments != 1){
        my_error("Error : more arguments than expected");
        return;
    }
    int id = atoi(array_of_substrings[0]);
    if( id < 0 || id >= job_no){
        my_error("Invalid job id");
        return;
    }
    JOB *job = get_job_for_jobid(id);
    if(job == NULL){
        my_error("Invalid job id");
        return;
    }
    debug("To implement  pause request : %s",line);
}

void resume_request(char *line){
    line  = line + 7;
    line = my_trim(line);
    if(line == NULL){
        my_error("Error : unexpected no of arguments");
        return;
    }
    int len = strlen(line);
    debug("New Length is : %d",len);
    char *array_of_substrings[len];
    int segments = update_array_of_substring(line,array_of_substrings);
    if(segments != 1){
        my_error("Error : more arguments than expected");
        return;
    }
    int id = atoi(array_of_substrings[0]);
    if( id < 0 || id >= job_no){
        my_error("Invalid job id");
        return;
    }
    JOB *job = get_job_for_jobid(id);
    if(job == NULL){
        my_error("Invalid job id");
        return;
    }
    debug("To implement  resume request : %s",line);
}
void disable_request(char *line){
    debug("To implement  disable request : %s",line);

    line  = line + 7;
    line = my_trim(line);
    if(line == NULL){
        my_error("Error : unexpected no of arguments");
        return;
    }
    int len = strlen(line);
    debug("New Length is : %d",len);
    char *array_of_substrings[len];
    int segments = update_array_of_substring(line,array_of_substrings);
    if(segments != 1){
        my_error("Error : more arguments than expected");
        return;
    }
    if( !printer_exist( array_of_substrings[0] )){
        my_error("Error : Printer does not exist");
        return;
    }

    int printer_id = printer_exist(array_of_substrings[0] ) -1;
    PRINTER *printer = printer_array[printer_id];
    if(printer->enabled == 0){
        my_error("Already Enabled");
        return;
    }
    printer->enabled  = 0;
    my_printer(printer);
    debug("To implement  enable request : %s",line);
    //check();
}
void enable_request(char *line){

    line  = line + 7;
    line = my_trim(line);
    if(line == NULL){
        my_error("Error : unexpected no of arguments");
        return;
    }
    int len = strlen(line);
    debug("New Length is : %d",len);
    char *array_of_substrings[len];
    int segments = update_array_of_substring(line,array_of_substrings);
    if(segments != 1){
        my_error("Error : more arguments than expected");
        return;
    }
    if( !printer_exist( array_of_substrings[0] ) ){
        my_error("Error : Printer does not  exist");
        return;
    }

    int printer_id = printer_exist(array_of_substrings[0] ) - 1;
    PRINTER *printer = printer_array[printer_id];
    if(printer->enabled == 1){
        my_error("Already Enabled");
        return;
    }
    printer->enabled  = 1;
    my_printer(printer);
    //debug("To implement  enable request : %s",line);
    //check();
    //printf("testing");
}

char *get_line(char *line_buffer,int in){
    if( in == 0 ){
        char *line;
        line = readline ("imp>");
        char *new_str = get_memory(strlen(line)+5);
        new_str = strcpy(new_str,line);
        free(line);
        return new_str;
    }else{
        //fscanf(in,"%[^\n]", line_buffer );
        input_over = scan_line(line_buffer);
        line_buffer = my_trim(line_buffer);
        char *new_str = get_memory(strlen(line_buffer)+5);
        new_str = strcpy(new_str,line_buffer);
        debug("line read :%s",new_str);
        return new_str;
    }
}

int scan_line(char *buffer){
    char c = getchar();
    int i = 0;
    while( c != '\n' && c != EOF ){
        //debug("char is :%c",c);
        buffer[i++] = c;
        c = getchar();
    }
    buffer[i] ='\0';
    if(c == EOF) return 1;
    debug("String is : %s",buffer);
    return 0;
}