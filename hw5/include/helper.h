#ifndef HELPER
#define HELPER

#include "debug.h"
#include "data.h"

void key_fini(MAP_ENTRY *entry);
void clean_blob(BLOB *blob);
void version_list_fini(VERSION *curr_version);
void clear_key_and_versions(MAP_ENTRY *curr_map_entry);
KEY *get_new_copied_key(KEY *old_key);
//MAP_ENTRY *get_map_entry_from_search_space(MAP_ENTRY * , KEY *);
void sortify_versions( MAP_ENTRY *map_entry );



/*
int get_no(char *str){
    int len = strlen(str);
    int no = 0;
    for(int i = 0; i < len ; i++){
        char c = str[i];
        if( c < '0' || c > '9')
            return -1;
        int digit = (int)(c-'0');
        no = no * 10 + digit;
        if(no > 65535)
            return -1;
    }
    return no;
}


void exit_cleanly(){
    debug("To implement : exit cleanly");
    //ToDo
    exit(EXIT_SUCCESS);
}

void exit_dirty(){
    debug("To implement : exit dirty");
    //ToDo
    exit(EXIT_FAILURE);
}
*/


int my_write( int fd, void *buff ,  int size ){
    int written = 0;
    int fill = write(fd,buff,size);
    if( fill < 0 ) return -1;
    written += fill;
    if(written == size) return size;
    while( written != size){
        fill = write(fd,buff + written , size - written);
        if(fill < 0) return -1;
        written += fill;
    }
    return size;
}

int my_read( int fd, void *buff ,  int size ){
    int written = 0;
    int fill = read(fd,buff,size);
    if( fill < 0 ) return -1;
    written += fill;
    if(written == size) return size;
    while( written != size){
        fill = read(fd,buff + written , size - written);
        if(fill < 0) return -1;
        written += fill;
    }
    return size;
}



#endif