

#ifndef MYMEM_H
#define MYMEM_H

#include "debug.h"

typedef struct memory {
    struct memory *next;
    void *ptr;
} MEMORY;

MEMORY *memory_head = NULL;
//MEMORY *memory_curr = NULL:


void insert_to_memory_queue(void *mem){
    MEMORY *data_head = malloc(sizeof(MEMORY));
    data_head->ptr = mem;
    data_head->next = memory_head;
    memory_head = data_head;
}

void remove_from_memory_queue(void *ptr_to_free){
    MEMORY *node = memory_head;
    MEMORY *prev = NULL;
    while(node != NULL){
        if(node->ptr == ptr_to_free){
            if(node == memory_head)
                memory_head = memory_head->next;
            else
                prev->next = node->next;
            free(node);
            return;
        }
        prev = node;
        node = node->next;
    }
    debug("Should not happen : not able to free");
}

void *get_memory(int size){
    void *mem = malloc(size);
    if(mem == NULL)
        mem = malloc(size);
    insert_to_memory_queue(mem);
    return mem;
}

void free_memory(void *ptr){
    free(ptr);
    remove_from_memory_queue(ptr);
}

void free_all(){
    MEMORY *node = memory_head;
    MEMORY *next = NULL;
    while(node != NULL){
        next = node->next;
        free_memory(node->ptr);
        node = next;
    }
}

#endif
