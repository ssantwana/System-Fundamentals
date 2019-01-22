#ifndef IMPLEMENTATION_H
#define IMPLEMENTATION_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "debug.h"
#include "my_lib.h"
#include "implementation.h"
#include <sys/types.h>
#include <sys/wait.h>
//#include "memory.h"

#include "imprimer.h"


int set_up_conversion_and_print(JOB *job, PRINTER *printer);
int copy( char *type_array[]);

int print_jobs( JOB *job , PRINTER *printer, char *type , int read_fd){
    debug("to print");
    return 0;
}

void check(){
    // To Do
    debug("Checking if print jobs can be executed");
    JOB *job = jobs_head;
    pid_t pid = 0;
    while( job  != NULL){

        if(job->status != QUEUED ){
            job = job->other_info;
            continue;
        }

        int result = get_my_conversion(job->file_type,job->eligible_printers);

        if( result == 0 ){
            job = job -> other_info;
            continue;
        }
        PRINTER *printer = printer_array[result - 1];
        printer->busy = 1;
        job->status = RUNNING;
        pid = fork();
        if(pid == 0){
            int success = set_up_conversion_and_print(job,printer);
            debug("Success after conversion : %d",success);
            debug("To Do");
            debug("According to handbook");
        }else{
            debug(" Main process waiting for child to finish ");
            debug("To do");
            debug("Dont need to wait in actual implementation");
        }
        printer->busy = 0;
        return;
    }

}

int execute_pipe( int in , int out , HASHMAP_CONVERSION *conversion_node ){
    int err = 0;
    pid_t pid;
    int status = -1;
    pid = fork();
    if( pid < 0)
        return pid;
    if(pid == 0){
        if(in != 0){
            err = dup2(in,0);
            if(err < 0)
                return err;
        }
        if(out != 1){
            err = dup2(out,1);
            if(err < 0)
                return err;
            close(out);
        }
        err = execvp(conversion_node->conversion_function,conversion_node->arguments);
        if(err < 0)
            return err;

    }else{
        sleep(2);
        debug("Parent process");
        waitpid(pid,&status,0);
        debug("child process : must be completed with pid : %d",pid);
        if(status < 0)
            return status;
    }
    return 0;
}

int seting_up_conversion_pipeline( JOB *job , char *type_array[] , int count){
    int in = 0, out = 0;
    in = std_in;
    int pipe_arr[2];
    int error = 0;
    for(int i = 0 ; i < count -1 ; i++){
        error = pipe(pipe_arr);
        out = i == count -2 ? std_out : pipe_arr[1];
        if( error  < 0)
            return error;
        out = pipe_arr[1];
        char *from = type_array[i];
        char *to = type_array[i+1];
        char *conversion_hash = create_conversion_hash(from,to);
        HASHMAP_CONVERSION *conversion_node = get_conversion_function( conversion_hash );
        error = execute_pipe( in , out , conversion_node );
        if(error < 0){
            debug("To Do");
            debug(" To implement error handling here ");
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
    int read_fd = 0;
    if(count == 1){
        if(strcmp(job->file_type,printer->type)  || strcmp(type_array[0],printer->type) ){
            debug("Issue must not happen location implementation.h : set_up_conversion_and_print");
            return 0;
        }
    }else{
        int read_fd = seting_up_conversion_pipeline(job , type_array , count);
        if( read_fd >= 0 ){
            debug("conversion_sucess");
        }else{
            debug("Conversion failure");
            debug("To Do Implement failure");
            return 0;
        }
    }
    int print_result = print_jobs( job , printer, type_array[count-1] , read_fd);
    debug("To Do print_result : %d",print_result);
    return 1;

}

int copy( char *type_array[]){
    int count = 0;
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
#endif