#ifndef MYLIB_H
#define MYLIB_H

#include "memory.h"
#include <string.h>
//#include "implementation.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "debug.h"
#include <sys/types.h>
#include <sys/wait.h>


#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "imprimer.h"

void check();

JOB *get_job_for_jobid(int id);
void printer_state_updated( PRINTER *printer );
void job_state_updated(JOB *job);
int set_up_conversion_and_print(JOB *job, PRINTER *printer);
int copy( char *type_array[]);

void my_error(char *issue){
    char buf[100];
    char *temp = imp_format_error_message(issue , buf , 100);
    printf("%s\n",temp);
    fflush(stdout);
}


void my_printer(PRINTER *printer){
    char buf[200];
    char *temp = imp_format_printer_status( printer , buf , 200 );
    printf("%s\n",temp);
    fflush(stdout);
    check();
}
void my_job(JOB *job){
    char buf[200];
    char *temp = imp_format_job_status( job , buf , 200 );
    printf("%s\n",temp);
    fflush(stdout);
    check();
}

int job_no = 1;
JOB *jobs_head = NULL;


JOB *finished_jobs[100];
void clear_finished_jobs(){
    struct timeval currtime;
    gettimeofday (&currtime, NULL);

    for(int i = 0 ; i < 100 ; i++){
        JOB *job = finished_jobs[i];
        if(job == NULL) continue;
        struct timeval job_time = job->change_time;
        if(currtime.tv_sec - job_time.tv_sec > 60)
            finished_jobs[i] = NULL;

    }

}



void add_to_finished_jobs(JOB *job){
    clear_finished_jobs();
    for(int i = 0 ; i < 100 ;i++){
        if( finished_jobs[i] == NULL ){
            finished_jobs[i] = job;
            return;
        }
    }
    debug("Cound not find slot in finished jobs slot");
}


void remove_from_jobs_queue(int job_id){
    JOB *curr_job = jobs_head;
    JOB *prev = jobs_head;
    if( jobs_head == NULL){
        debug("Could not remove job from queue");
        return;
    }
    JOB *test = jobs_head;
    if(test->jobid == job_id){
        jobs_head = test->other_info;
        return;
    }
    while(curr_job != NULL){
        if(curr_job->jobid == job_id){
            prev->other_info = curr_job->other_info;
            return;
        }
        prev = curr_job;
        curr_job = curr_job->other_info;
    }
    debug("Could not remove job from queue");
}

typedef struct my_q {
    struct my_q *prev;
    struct my_q *next;
    char *type;  /* You may store other info in this field. */
} QUEUE_DATA;

char printer_buffer[200];
char job_buffer[200];
//int std_in = 0;
//int std_out = 1;
QUEUE_DATA *head_queue = NULL;
QUEUE_DATA *last_node = NULL;


int no_of_printers = 0;
PRINTER * printer_array[32];

QUEUE_DATA *create_q_data(char *data){
    QUEUE_DATA *data_ptr = get_memory(sizeof(QUEUE_DATA));
    data_ptr->type = data;
    data_ptr->prev = NULL;
    data_ptr->next = NULL;
    return data_ptr;
}

void remove_q_data(){
    QUEUE_DATA *node = last_node;
    if(node == NULL){
        debug("Must never happen");
    }
    last_node = last_node->prev;
    free_memory(node);
    //free_memory(ptr);
}





void testing_header(){
    printf("\n=====testing header=====\n");
}

void add_to_queue(char *data){
    QUEUE_DATA *node = create_q_data(data);
    debug("Adding type : %s",data);
    if(head_queue == NULL){
        head_queue = node;
        last_node = node;
        node->prev = head_queue;
        debug("Adding first type : %s",data);
        return;
    }
    node->prev = last_node;
    node->next = NULL;
    last_node->next = node;
    last_node = node;
    debug("Adding other type : %s",data);
}

int already_visited(char *S1){
    int true_flag = 1;
    int false_flag = 0;
    QUEUE_DATA *node = head_queue;
    while(node != NULL){
        char *S2 = node->type;
        if(strcmp(S1,S2) == 0)
            return true_flag;
        node = node->next;
    }
    return false_flag;

}

typedef struct hm_value {
    //struct my_q *prev;
    struct hm_value *next;
    char *type_value;  /* You may store other info in this field. */
} HASHMAP_VALUE;

typedef struct hm_key {
    //struct my_q *prev;
    struct hm_value *value_head;
    struct hm_key *next;
    char *type_key;  /* You may store other info in this field. */
} HASHMAP_KEY;

/*
typedef struct job_extra_info {
    int job_pid;
    JOB *next;
} JOB_EXTRA_INFO;
*/

HASHMAP_KEY *hashmap_head = NULL;

void insert_to_hash_map(char *from , char *to){
    HASHMAP_KEY *key = hashmap_head;
    while(key != NULL){
        if (strcmp(from,key->type_key) == 0)
        {
            HASHMAP_VALUE *new_value = get_memory(sizeof(HASHMAP_VALUE));
            new_value->type_value = to;
            new_value->next = key->value_head;
            key->value_head = new_value;
        }
        key = key->next;
    }
    HASHMAP_KEY *new_key = get_memory(sizeof(HASHMAP_KEY));
    HASHMAP_VALUE *new_value = get_memory(sizeof(HASHMAP_VALUE));
    new_value->type_value = to;
    new_value->next = NULL;
    new_key->value_head = new_value;
    new_key->type_key = from;
    new_key->next = hashmap_head;
    hashmap_head = new_key;
    debug("Adding to hashmap from : %s to : %s ",from,to);
}


HASHMAP_VALUE *get_values(char *from){
    HASHMAP_KEY *node = hashmap_head;
    while(node != NULL){
        if(strcmp(from,node->type_key) == 0)
            return node->value_head;
        debug("this type : %s does not match : %s",from,node->type_key);
        node = node->next;
    }
    return NULL;
}

int length_of_queue(){
    int count = 0;
    QUEUE_DATA *q_node = head_queue;
    debug("calculating length of Q");
    while( q_node  != NULL){
        debug("type : %s",q_node->type);
        count++;
        q_node = q_node->next;
        //free_memory(temp);
    }
    return count;
}

int acceptable_type(char *curr, PRINTER_SET set){
    debug("Going to search for all printers");
    for(int i = 0 ; i < no_of_printers ; i++){
        PRINTER *printer = printer_array[i];
        if(strcmp(printer->type,curr) == 0 && printer->enabled == 1 && printer->busy == 0){
            debug("Printer : %s fits the job",printer->name);
            length_of_queue();
            return printer->id + 1;
        }
        debug("printer :%s with id %d is not compatible",printer->name , printer->id);
    }
    return 0;
}


int my_DFS(char *curr , PRINTER_SET set){
    int printer_for_current_type = acceptable_type(curr,set);
    if( printer_for_current_type ) return printer_for_current_type;
    //debug("type : %s is not target",curr);
    HASHMAP_VALUE *q = get_values(curr);
    /*if( q == NULL){
        debug(" No wat to convert for : %s",curr);
        if( hashmap_head == NULL){
            debug("hashmap_head is null");
        }else{

        }
    }*/
    while( q != NULL ){
        char *curr_node = q->type_value;
        if(already_visited(curr_node)){
            q = q->next;
            continue;
        }
        add_to_queue(curr_node);
        int result = my_DFS(curr_node,set);
        if( result ) return result;
        remove_q_data();
        q = q->next;
    }
    return 0;
}




int get_my_conversion_pipeline_for_types(char *start , PRINTER_SET set){
    add_to_queue(start);
    int result = my_DFS(start,set);
    if(result){
        debug("positive result of DFS : %d\n",result);
        return result;
    }
    head_queue = NULL;
    last_node = NULL;
    //remove_q_data();
    debug("DFS :No path found");
    return 0;
}



// Type_List


typedef struct type_list {
    //struct my_q *prev;
    struct type_list *next;
    char *type;  /* You may store other info in this field. */
} TYPE_LIST;



TYPE_LIST *type_list_head = NULL;

int type_exist(char *type){
    TYPE_LIST *node = type_list_head;
    while( node != NULL ){
        if( strcmp(type,node->type) == 0 )
            return 1;
        node = node->next;
    }
    debug("Failed to find type : %s",type);
    return 0;
}

int add_type_to_list(char *new_type){
    if( type_exist(new_type) == 1)
        return 0;
    TYPE_LIST *node = get_memory(sizeof(TYPE_LIST));
    node->type = new_type;
    node->next = type_list_head;
    type_list_head = node;
    return 1;
}



//            Conversion HashMAP




typedef struct converion_func {
    struct converion_func *next;
    char *conversion_hash;  /* You may store other info in this field. */
    char *conversion_function;
    char **arguments;
    int count_arg;
} HASHMAP_CONVERSION;

HASHMAP_CONVERSION *head_conversion = NULL;

int conversion_hash_already_exist(char *conversion_hash){
    HASHMAP_CONVERSION *node = head_conversion;
    while( node != NULL ){
        if(strcmp(conversion_hash,node->conversion_hash) == 0)
            return 1;
        node = node->next;
    }
    return 0;
}


char *create_conversion_hash(char *from , char *to){
    int len = strlen(from) + strlen(to) + 5;
    char *conversion_hash = get_memory(len * sizeof(char));
    strcpy(conversion_hash,from);
    strcat(conversion_hash,":");
    strcat(conversion_hash,to);
    return conversion_hash;
}

int add_converter(char *from , char *to , char *converter , char* arr[] , int count ){
    int len = strlen(from) + strlen(to) + 5;
    char *conversion_hash = get_memory(len * sizeof(char));
    strcpy(conversion_hash,from);
    strcat(conversion_hash,":");
    strcat(conversion_hash,to);
    if(conversion_hash_already_exist(conversion_hash)){
        // To Do handle error;
        debug("Need to implement error handling");
        printf("Error : conversion already exist");
        return 0;
    }
    HASHMAP_CONVERSION *new_converter = get_memory(sizeof(HASHMAP_CONVERSION));
    new_converter->conversion_hash = conversion_hash;
    new_converter->conversion_function = converter;
    new_converter->next = head_conversion;
    head_conversion = new_converter;
    new_converter->arguments = arr + 3;
    new_converter->count_arg = count - 3;
    insert_to_hash_map(from,to);
    debug("Adding conversion");
    return 0;
}


HASHMAP_CONVERSION *get_conversion_function(char *conversion_hash){
    HASHMAP_CONVERSION *node = head_conversion;
    while( node != NULL ){
        if(strcmp(conversion_hash,node->conversion_hash) == 0)
            return node;
        node = node->next;
    }
    return NULL;
}

char *my_trim(char *orignal){
    //char *new_string = orignal;
    debug("line: %s",orignal);
    if(orignal == '\0') return NULL;
    int len = strlen(orignal);
    int start = 0;
    for(int  i = 0 ; i < len ; i++){
        start = i;
        if(orignal[i] != ' ')
            break;
    }
    debug("start : %d and len : %d",start,len);
    if(start == len - 1 && orignal[start] == ' ')
        return NULL;
    int end  = len -1;
    for(int  i = len -1 ; i >=  0 ; i--){
        end = i;
        if(orignal[i] != ' ')
            break;
    }
    orignal[end+1] = '\0';
    return orignal + start;
}


int update_array_of_substring(char *line, char *array_of_substrings[]){
    int index = 0;
    int len = strlen(line);
    array_of_substrings[0] = line;
    index = 1;
    debug("line : %s of length :%d",line,len);
    for(int i = 0 ; i < len - 1 ; i++){
        //debug("line[i] : %d",(int)line[i]);
        if(((int)(line[i]) == 32 || line[i] == 0)  && (int)(line[i+1]) != 32){
            debug("reached here :%d",i);
            array_of_substrings[index++] = line + i + 1;
        }
        if( (int)(line[i]) != 32 && (int)(line[i+1]) == 32){
            debug("reached here 2");
            line[i+1] = '\0';
        }
    }
    debug("index :%d",index);
    debug("about to segmentation fault");
    array_of_substrings[index] = NULL;
    debug("after segmentation");
    return index;
}



// Initalizing Printer Array


void initialize_printer_array(PRINTER *printer_array[]){
    for(int i = 0 ; i < 32 ; i++)
        printer_array[i] = NULL;

}




int printer_exist(char *printer_name){
    for(int i = 0 ; i < no_of_printers ; i++){
        PRINTER *printer = printer_array[i];
        if(strcmp(printer->name,printer_name) == 0 )
            return printer->id + 1;
    }
    return 0;
}


PRINTER *add_printer(char *printer_name , char *printer_type){
    PRINTER *new_printer = get_memory(sizeof(PRINTER));
    new_printer->name = printer_name;
    new_printer->type = printer_type;
    new_printer->id = no_of_printers;
    new_printer->enabled = 0;
    new_printer->busy = 0;
    new_printer->other_info = NULL;
    printer_array[no_of_printers] = new_printer;
    no_of_printers++;
    return new_printer;
    debug("Printer name after adding : %s",new_printer->name);
}





JOB *add_job(char *file , PRINTER_SET eligible_printers ,char *file_type){
    JOB *new_job = get_memory(sizeof(JOB));
    new_job->jobid = job_no++;
    new_job->status = QUEUED;
    new_job->file_name = file;
    new_job->file_type = file_type;
    new_job->eligible_printers = eligible_printers;
    new_job->chosen_printer = NULL;
    gettimeofday (&(new_job->change_time), NULL);
    gettimeofday (&(new_job->creation_time), NULL);
    // To Do update time
    //new_job->creation_time = get_time();
    //new_job->change_time = get_time();
    //JOB_EXTRA_INFO *job_extra_info = get_memory(JOB_EXTRA_INFO);
    //job_extra_info->next = jobs_head;
    //new_job->other_info = job_extra_info;
    new_job->other_info = (void *)jobs_head;
    jobs_head = new_job;
    return new_job;
}

char *get_file_type_from_file(char *file){
    //char *type = NULL;
    int len = strlen(file);
    for(int i = len -1  ; i > 0 ; i--){
        if(file[i] == '.'){
            return file + i + 1;
        }
    }
    return NULL;
}



PRINTER_SET get_printer_set(int segments, char **array_of_substrings){
    PRINTER_SET set;
    for(int i = 0; i < segments ; i++){
        char *printer = array_of_substrings[i];
        debug("checking in printer : %d",i);
        int printer_id = printer_exist(printer) - 1;
        if(printer_id == -1){
            return 0;
        }
        if(((set>>printer_id)&1) == 1){
            debug("same printer coming again");
            return 0;
        }
        set = set^(1<<printer_id);
    }
    return set;

}

JOB *completed_jobs_head = NULL;

void update_complete_job_state(int pid, int status){

    JOB *job = jobs_head;
    debug("here to update job status");
    while(job != NULL){
        //JOB_EXTRA_INFO *exta_info = job->other_info;
        if(job->status != RUNNING || job->pgid != pid  ){
            job = job->other_info;
            continue;
        }
        debug("Updating info for job");
        PRINTER *printer = job->chosen_printer;
        debug("Printer : %p",(void *)printer);
        printer->busy = 0;
        debug("Before printer status updated");
        printer_state_updated(printer);
        job->status = status == 0 ? COMPLETED : ABORTED;
        gettimeofday (&(job->change_time), NULL);
        debug("Comment me and job status : ");
        job_state_updated(job);
        gettimeofday (&(job->change_time), NULL);
        add_to_finished_jobs(job);
        remove_from_jobs_queue(job->jobid);
        //my_printer(printer);
        //my_job(job);
        debug("Update info complete");
        return;

    }

}

void printer_state_updated( PRINTER *printer ){
    char buf[1000];
    char *temp = imp_format_printer_status(printer , buf, 1000);
    printf("%s\n",temp);

}

void job_state_updated(JOB *job){
    char buf[1000];
    char *temp = imp_format_job_status( job , buf , 1000 );
    printf("%s\n",temp);
}

void check(){
    // To Do
    debug("Checking if print jobs can be executed");
    JOB *job = jobs_head;
    pid_t pid = 0;
    while( job  != NULL){

        if(job->status != QUEUED ){
            job = job -> other_info;
            continue;
        }
        debug("checking for jobid:%d",job->jobid);
        //clear_queue_for_DFS();
        head_queue = NULL;
        last_node = NULL;
        int result = get_my_conversion_pipeline_for_types(job->file_type,job->eligible_printers);

        if( result == 0 ){
            job = job -> other_info;
            continue;
        }
        PRINTER *printer = printer_array[result - 1];
        printer->busy = 1;
        job->status = RUNNING;
        job->chosen_printer = printer;
        gettimeofday (&(job->change_time), NULL);
        my_job(job);
        my_printer(printer);
        pid = fork();
        if(pid == 0){
            pid = getpid();
            setpgid(pid,pid);
            //sleep(10);
            int success = set_up_conversion_and_print(job,printer);
            if(!success){
                kill(getpid(),SIGKILL);
            }
            debug("Child : Success after conversion : %d",success);
            debug("To Do");
            debug("According to handbook");
            exit(0);
        }else{
            //JOB_EXTRA_INFO *job_extra_info = job->other_info;
            //job_extra_info->job_pid = pid;
            job->pgid = pid;
            //my_job(job);
            debug("=================Going to sleep");
            //sleep(10);
            debug("=================Waiking from sleep");
            debug("Parent : parent id : %d child id : %d",getpid(),pid);
            debug(" Main process waiting for child to finish ");
            debug("To do");
            debug("Dont need to wait in actual implementation");
        }
        //printer->busy = 0;
        //return;
        job = job->other_info;
    }

}







//void check();

int print_jobs( JOB *job , PRINTER *printer, char *type , int read_fd){
    int fd = imp_connect_to_printer( printer, 0 );
    if(fd < 0 ){
        killpg(getpgid(getpid()),SIGKILL);
        exit(1);
    }
    debug("fd  for printer :%d",fd);
    FILE *fp_read = fdopen(read_fd , "r");
    FILE *fd_write = fdopen(fd , "w");
    debug(" pointer : %p pointer : %p",(void *)fp_read , (void *)fd_write);
    char c = fgetc(fp_read);
    while(c != EOF){
        fputc(c,fd_write);
        c  = fgetc(fp_read);
    }
    debug("Printer writter");
    fclose(fp_read);
    fclose(fd_write);


    debug("to print");
    exit(EXIT_SUCCESS);
    debug("$$$$$$$$$$$$$ Should Not Happen$$$$$$$$$$$$$$$$$$");
    return 1;
}

void handler(int sig){
    int  status,pid;
    debug("\nHandler : Signal no : %d\n",sig);
    debug("Handler : =====Handler hit====\n");
    switch(sig){
        case 20:
        case 17:
        case 18:
        pid = waitpid(-1, &status, 0);
        debug("Handler : Signal recieved frrom pid : %d my pid : %d\n",pid,getpid());
        update_complete_job_state(pid,WEXITSTATUS(status));
        debug("Status : %d and exit_status:\n",status);
        debug("test\n");


        //return;
        break;
    }
    printf("Handler over\n");

}
void clear_queue_for_DFS(){
    QUEUE_DATA *q_node = head_queue;
    debug("head_queue : %p",(void *)head_queue);
    while( q_node  != NULL){
        QUEUE_DATA *temp = q_node;
        debug("Saving to temp");
        q_node = q_node->next;
        debug("Before free ");
        free_memory(temp);
    }
    head_queue = NULL;
    last_node = NULL;
}


int execute_pipe( int in , int out , HASHMAP_CONVERSION *conversion_node ){
    int err = 0;
    pid_t pid;
    int status = -1;
    debug("executing pipe");
    pid = fork();
    if( pid < 0){
        killpg(getpgid(getpid()),SIGKILL);
        return pid;
        //killpg(getpgid(),SIGKILL);
    }
    debug("Fork success for one conversion of pipe");
    if(pid == 0){
        if(in != 0){
            err = dup2(in,0);
            if(err < 0){
                killpg(getpgid(getpid()),SIGKILL);
                return err;
                //killpg(getpgid(),SIGKILL);
            }
        }
        if(out != 1){
            err = dup2(out,1);
            if(err < 0){
                return err;
            }
            close(out);
        }

        debug("Child : About to exec");
        err = execvp(conversion_node->conversion_function,conversion_node->arguments);
        if(err < 0){
            killpg(getpgid(getpid()),SIGKILL);
            debug("Exec failure");
            return err;
        }

    }else{
        //sleep(2);
        debug("Parent process");
        waitpid(pid,&status,0);
        debug("child process : must be completed with pid : %d",pid);
        if(status < 0){
            killpg(getpgid(getpid()),SIGKILL);
            return status;
        }
    }
    return 0;
}

int seting_up_conversion_pipeline( JOB *job , char *type_array[] , int count){
    int in = 0, out = 0;
    //in = std_in;
    int pipe_arr[2];
    int error = 0;
    in = open(job->file_name,O_RDONLY);
    if(in  < 0){
        debug("unable to open file");
        return in;
    }
    for(int i = 0 ; i < count -1 ; i++){
        error = pipe(pipe_arr);
        out = pipe_arr[1];
        char *from = type_array[i];
        char *to = type_array[i+1];
        char *conversion_hash = create_conversion_hash(from,to);
        HASHMAP_CONVERSION *conversion_node = get_conversion_function( conversion_hash );
        error = execute_pipe( in , out , conversion_node );
        if(error < 0){
            debug("Child : To Do");
            debug("Child : To implement error handling here ");
            return error;
        }
        in = pipe_arr[0];
        close(pipe_arr[1]);
    }
    return pipe_arr[0];
}

int set_up_conversion_and_print(JOB *job, PRINTER *printer){
    //QUEUE_DATA *q = head_queue;
    char *type_array[2056];
    int count = copy(type_array);
    debug("Child : Size of type_array:%d",count);
    int read_fd = open(job->file_name ,O_RDONLY);
    if(read_fd < 0){
        debug("Child : TO Do");
        debug("Child : FI file fail to open");
        debug("");
        return 0;
    }else{
        debug("Child : File open successfully");
    }
    if(count == 1){
        if(strcmp(job->file_type,printer->type)  || strcmp(type_array[0],printer->type) ){
            debug("Child : Issue must not happen location implementation.h : set_up_conversion_and_print");
            return 0;
        }
        debug("Child : ===No need of piping==");
    }else{
        read_fd = seting_up_conversion_pipeline(job , type_array , count);
        if( read_fd >= 0 ){
            debug("Child : conversion_sucess");
        }else{
            debug("Child : Conversion failure");
            debug("Child : To Do Implement failure");
            return 0;
        }
    }
    print_jobs( job , printer, type_array[count-1] , read_fd);
    //debug("Child : To Do print_result : %d",print_result);
    return 1;

}


int copy( char *type_array[]){
    int count = 0;
    debug("Expected length of type q is : %d",length_of_queue());
    QUEUE_DATA *q_node = head_queue;
    while( q_node  != NULL){
        type_array[count++] = q_node->type;
        QUEUE_DATA *temp = q_node;
        q_node = q_node->next;
        free_memory(temp);
    }
    head_queue = NULL;
    last_node = NULL;
    return count;
}


JOB *get_job_for_jobid(int id){
    JOB *job = jobs_head;
    while(job != NULL){
        if(job->jobid == id){
            return job;
        }
        job = job->other_info;
    }
    return NULL;
}

#endif