/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include <errno.h>
//#include <stderr.h>

//# define EINVAL 22
//# define ENOMEM 12



int invalid_ptr(void *ptr);
void free_valid_ptr(sf_header *header);
void *get_free_block(size_t );
void update_prologue_epilogue(void *);
void update_prologue();
void update_epilogue();
void add_to_free_list(sf_header *, int );
void insert_to_free_list(sf_header *head , int size);
sf_header *pre_coales( sf_header * pre_block ,sf_header * post_block , int pre_block_size , int post_block_size);
sf_header *post_coales( sf_header * pre_block ,sf_header * post_block , int pre_block_size , int post_block_size);
sf_header *search_free_block(int total);
void free_from_free_list(sf_header *pre_block,int pre_block_size);
void set_prev_allocated(void *, int);




void *upper_limit = NULL;
void *lower_limit = NULL;

void set_prev_allocated(void *block_start, int allocated){
    sf_block_info *header = block_start;
    int size = header->block_size * 16;
    sf_block_info *footer = block_start + (size - 8);
    header->prev_allocated = allocated;
    if(header->allocated == 0){
        footer->prev_allocated = allocated;
    }
}

void *sf_malloc(size_t size) {
    if(size == 0)
        return NULL;

    if(size < 0){
        sf_errno = EINVAL;
        return NULL;
    }
    debug("GOing to look for block for size request : %d",(int)size);

    void *free_block_header = get_free_block(size);

    // Not possible to get memory
    if(free_block_header == NULL){
        sf_errno = ENOMEM;
        return NULL;
    }
    debug("MALLOC FINISHED : final header_start : %p ",(void *)free_block_header);
    //sf_show_free_lists();
    debug("\n\n\n");
    //sf_show_block_info((void *)free_block_header);

    return  free_block_header + 8;

}

void sf_free(void *pp) {
    if(invalid_ptr(pp) == -1){
        sf_errno = EINVAL;
        debug("Invalid ptr aborting");
        abort();
        return;
    }
    //sf_show_free_list();
    debug("to free address : %p",pp);
    debug("upper_limit : %p lower_limit : %p",upper_limit,lower_limit);
    sf_header *header = pp - 8;
    free_valid_ptr(header);
}

void *sf_realloc(void *pp, size_t rsize) {
    if(invalid_ptr(pp) == -1 ){
        sf_errno = EINVAL;
        abort();
        return NULL;
    }
    if(rsize == 0){
        sf_free(pp);
        return NULL;
    }
    sf_block_info *info = pp - 8;
    int block_size = info->block_size * 16;
    int prev_requested_size = info->requested_size;
    if(rsize == prev_requested_size)
        return pp;
    int avaliable_size = block_size - 8;
    if(rsize <= avaliable_size){
        debug("convert to smaller block : avaliable_size : %d and rsize : %d",avaliable_size ,(int)rsize);
        sf_block_info *header =  pp - 8;
        int actual_rsize = rsize + (rsize + 8) % 16 == 0 ? 0 : 16 - ((rsize + 8) % 16);
        if(actual_rsize <= 24) actual_rsize = 24;
        int remaining_size = avaliable_size - actual_rsize;
        if(remaining_size >= 32){
            debug("if remaining block is big can be simply freed  as remaining_size : %d",remaining_size);
            header->block_size = (actual_rsize + 8) / 16;
            header->requested_size = rsize;
            header->allocated = 1;
            debug("Inserting size : %d",actual_rsize);

            sf_block_info *header_free = (void *)pp + ( actual_rsize );
            header_free->prev_allocated = 1;
            header_free->allocated = 0;
            header_free->block_size = remaining_size/16;
            free_valid_ptr((void *)header_free);
            return pp;
        }else{
            debug("When remaining block is small as remaining_size : %d",remaining_size);
            //sf_block_info *next_block = (void *)pp + (avaliable_size);
            header->requested_size = rsize;
            return pp;
        }
    }else{
        // Manage data
        void *result_ptr = sf_malloc(rsize);
        if(result_ptr == NULL){
            sf_errno = ENOMEM;
            return NULL;
        }
        memcpy(result_ptr,pp,prev_requested_size);
        sf_free(pp);
        return result_ptr;
    }
    return NULL;
}


void free_valid_ptr(sf_header *header){
    debug("free valid ptr : %p",header);
    sf_block_info *header_info = &(header->info);
    header_info->allocated = 0;
    header_info->requested_size = 0;
    int total_size = header_info->block_size * 16;
    if(header_info->prev_allocated == 0 && header != upper_limit){
        if(header != upper_limit){
            debug("Will be doing pre_coales");
            sf_block_info *prev_block_info = (void *)header_info - 8;
            int prev_block_size = prev_block_info->block_size * 16;
            sf_header *prev_block_start_pos = (void *)header_info - prev_block_size;
            pre_coales(prev_block_start_pos,header,prev_block_size,header_info->block_size * 16);
            header_info = (void *)prev_block_start_pos;
            total_size = header_info->block_size * 16;
        }else{
            debug("Must never happen : prev allocated");
        }
    }
    debug("header_info : %p total_size : %d",header_info,total_size);
    sf_block_info *post_block = (void *)header_info + total_size;
    debug("post_block : %p",post_block);
    if(post_block->allocated == 0){
        post_coales((void *)header_info,(void *)post_block , total_size , post_block->block_size * 16);
        total_size = header_info->block_size * 16;
    }

    insert_to_free_list((void *)header_info , total_size);

    return;
}

int invalid_ptr(void *ptr){
    // To Do
    if(ptr == NULL || ptr < upper_limit || ptr >= lower_limit  || ptr < sf_mem_start() + 32 ){
        debug("Invalid ptr :  fail1");
        return -1;
    }
    sf_block_info *info = ptr - 8 ;
    if(info->allocated == 0 || info->block_size * 16 < 32 || info->requested_size + 8 > (info->block_size * 16 )) {
        //sf_show_block_info(ptr);
        debug("Invalid ptr :  fail2");
        return -1;
    }
    sf_block_info *prev_block_footer = ptr - 16;
    if( info->prev_allocated == 0 && prev_block_footer->allocated == 1 ){
        //sf_show_block_info(ptr-8);
        //sf_show_block_info(ptr-16);
        debug("Invalid ptr :  fail3");
        return -1;
    }
    return 0;
}

void *get_free_block(size_t requested_memory ){
    debug("get_free_block");
    size_t total = requested_memory;

    sf_header *head = search_free_block(total);
    while(head == NULL){
        debug("no free_block : trying to grow mem");
        void *new_block = sf_mem_grow();
        debug("new_block size : %p   start_mem : %p  end_mem : %p  and size of memory : %d"
            ,new_block ,sf_mem_start(), sf_mem_end(),(int)(sf_mem_end()-new_block));
        debug("diffrence : %d",(int)(sf_mem_end() - sf_mem_start()));
        if(new_block == NULL || (int)(sf_mem_end()-new_block) != PAGE_SZ){
            sf_errno = ENOMEM;
            return NULL;
        }
        //sf_block_info header = {}
        update_prologue_epilogue((void *)new_block);
        head = search_free_block(total);
    }
    debug("head : %p",head);


    sf_block_info *header = (sf_block_info *)head;

    int size_got = header->block_size * 16;
    debug("size: got : %d , total : %d",size_got,(int)total);
    int extra_size = size_got  - (total + 8);
    if(total + 8 <= 32){
        extra_size = size_got - 32;
    }
    int free_block = 0;

    // To set prev allocated block
    //sf_block_info *next_frame_info = (void *)head + requested_memory;
    //next_frame_info->prev_allocated = 1;

    debug("extra size : %d",extra_size);
    if(extra_size >= 32){
        int multiple = extra_size / 16;
        free_block = multiple * 16;
        debug("free_block : %d multiple : %d",free_block , multiple);

        // Free Block
        sf_header *free_header = (void *)header + (size_got - free_block);
        sf_block_info temp;

        // Setting header for free_header
        free_header->links.next = NULL;
        free_header->links.prev = NULL;
        temp.allocated = 0;
        temp.prev_allocated = 1;
        temp.two_zeroes = 0;
        temp.block_size = multiple;
        free_header->info = temp;

        sf_block_info *free_footer = (void *)free_header + (free_block - 8);
        *free_footer = temp;

        insert_to_free_list(free_header,free_block);
        //next_frame_info->prev_allocated = 0;
    }
    size_got -= free_block;
    header->allocated = 1;
    header->two_zeroes = 0;
    header->block_size = size_got/16;
    header->requested_size = requested_memory;

    sf_block_info *next_block_info = (void *)header + size_got;
    next_block_info->two_zeroes = 0;
    set_prev_allocated(next_block_info , 1);
    return (void *)header;
}

void update_prologue_epilogue(void *head_new_block){
    if(head_new_block == sf_mem_start()){
        debug("update_prologue_epilogue : first time block");
        update_prologue();
        sf_block_info temp = {0,0,0,0,0};
        temp.allocated = 0;
        temp.block_size = (PAGE_SZ-48) / 16;
        temp.prev_allocated = 1;
        sf_block_info *header = head_new_block + 40;
        *header = temp;
        sf_block_info *footer = head_new_block + (PAGE_SZ - 8);
        *footer = temp;
        add_to_free_list((void *)head_new_block + 40, PAGE_SZ - 48);
        debug("Added to free list");
        update_epilogue();
        upper_limit = sf_mem_start() + 40;
        lower_limit = sf_mem_end() - 8;
        debug("epilogue updated");
    }else{
        debug("update_prologue_epilogue : allocate block PAGE_SZ");
        int my_size = PAGE_SZ;
        sf_block_info last_epilog = *((sf_block_info *)(head_new_block - 8));
        sf_block_info *my_header = head_new_block-8;
        if(last_epilog.prev_allocated == 0){
            sf_block_info prev_of_epilog = *((sf_block_info *)(head_new_block - 16));
            int block_size = prev_of_epilog.block_size;
            block_size = block_size * 16;
            debug("block size of pre_block : %d",block_size);
            debug("head_new_block : %p",(void *)head_new_block);
            sf_header *pre_block = ((void *)head_new_block) - (8 + block_size);
            debug("pre_block : %p",(void *)pre_block);
            my_header = (void *)pre_coales( pre_block , (sf_header *)(head_new_block-8) , block_size , PAGE_SZ);
            my_size += block_size;
        }
        debug("my header update_prologue_epilogue : %p and my_size : %d",my_header,my_size);
        sf_block_info temp = {0,0,0,0,0};
        temp.allocated = 0;
        temp.block_size = (my_size) / 16;
        temp.prev_allocated = 1;
        temp.requested_size = 0;
        sf_block_info *header = my_header;
        *header = temp;
        sf_block_info *footer = (void *)my_header + (my_size  - 8);
        debug("Footer to be set in free block at address : %p",(void *)footer);
        *footer = temp;
        debug("Footer set in free block");
        sf_header *sf_my_header = (void *)my_header;
        sf_my_header->links.next = NULL;
        sf_my_header->links.prev = NULL;
        debug("my header : %p",sf_my_header);
        add_to_free_list((void *)sf_my_header, my_size);
        update_epilogue();
        lower_limit = sf_mem_end() - 8;
    }
}

void add_to_free_list(sf_header *head, int size){
    if(size < 32 || size % 16  != 0)
        debug("Issue while adding to free list");
    insert_to_free_list(head , size );
}

void insert_to_free_list(sf_header *head , int size){
    // block_size is size of payload that it can provide
    debug("func : insert_to_free_list");
    debug("size in header info : %d and size: %d",head->info.block_size,size);
    debug("block ptr :being inserted :%p",(void *)head);
    //sf_show_free_lists();
    int block_size = size - 8;
    sf_block_info *temp_free = (void *)head;
    temp_free->allocated = 0;
    temp_free->prev_allocated = 1;
    temp_free->two_zeroes = 0;
    temp_free->block_size = size/16;
    temp_free->requested_size = 0;

    temp_free = (void *)head + size - 8;
    temp_free->allocated = 0;
    temp_free->prev_allocated = 1;
    temp_free->two_zeroes = 0;
    temp_free->block_size = size/16;
    temp_free->requested_size = 0;

    sf_block_info *next_block_info = (void *)head + size;
    next_block_info->prev_allocated = 0;

    debug("size in header info : %d",head->info.block_size);
    //sf_header *head_in_sf_header_format = head;

    sf_free_list_node *node = sf_free_list_head.next;

    while( node !=  &sf_free_list_head){
        // if exact block size found
        if(node->size - 8 < block_size){
            debug("Smaller block : met block of size : %d",(int)node->size);
            node = node->next;
            continue;
        }
        if(node->size - 8 > block_size){
            debug("Big Block found : %d",(int)node->size - 8);
            // Need to insert a sf_free_list_node here
            node = sf_add_free_list(block_size + 8,node);
            //node->size = block_size + 8;
            sf_header *temp_list = &(node->head);
            //sf_header *temp_list_addr = &(node->head);
            temp_list->links.next = temp_list;
            temp_list->links.prev = temp_list;
        }
        sf_header *start_node = &(node->head);
        sf_header *next_node = start_node->links.next;


        // inserting head in the first position
        head->links.next = next_node;
        head->links.prev = &(node->head);
        next_node->links.prev = head;
        start_node->links.next = head;
        debug("Node perfectly inserted to free list");
        //sf_show_free_lists();
        return;

    }
    debug("Inserting at end");
    node = sf_add_free_list(block_size + 8,node);
    //node->size = block_size + 8;
    sf_header *temp_list = &(node->head);
    temp_list->links.next = temp_list;
    temp_list->links.prev = temp_list;
    sf_header start_node = node->head;
    sf_header *next_node = start_node.links.next;


    // inserting head in the first position
    head->links.next = next_node;
    head->links.prev = &(node->head);
    next_node->links.prev = head;
    node->head.links.next = head;
    debug("Adding free list in empty list");
    //sf_show_free_lists();

}


void update_prologue(){
    sf_block_info *head  = sf_mem_start();
    sf_block_info padding  = {0,0,0,0,0};
    sf_block_info block  = {0,0,0,0,0};
    block.allocated = 1;

    *head = padding;

    head = (void *)head + 8;
    *head = block;


    head = (void *)head + 8;
    *head = padding;


    head = (void *)head + 8;
    *head = padding;


    head = (void *)head + 8;
    *head = block;

}


void update_epilogue(){
    sf_block_info *foot = sf_mem_end();
    sf_block_info block  = {0,0,0,0,0};
    block.allocated = 1;
    block.prev_allocated = 0;
    foot = (void *)foot - 8;
    *foot = block;
}


sf_header *pre_coales( sf_header * pre_block ,sf_header * post_block , int pre_block_size , int post_block_size){
    debug("pre_coales : pre_block_size : %d post_block_size : %d",pre_block_size , post_block_size);
    debug("pre_coales : pre_block_addr : %p post_block_addr : %p",pre_block , post_block);
    if(pre_block->info.block_size * 16 != pre_block_size){
        debug("pre coales : Inonsistent pre_block size : in header : %d and we are expecting : %d" ,
         (pre_block->info.block_size * 16) , pre_block_size);
    }
    free_from_free_list(pre_block,pre_block_size);
    int total_size = pre_block_size + post_block_size;
    sf_block_info info = {0,0,0,0,0};
    info.allocated = 0;
    info.prev_allocated = 1;
    info.two_zeroes = 0;
    info.block_size = total_size/16;
    info.requested_size = 0;
    pre_block->info = info;
    sf_block_info *last_pos = (void *)pre_block + (total_size - 8);
    *last_pos = info;
    debug("pre_coales returning : %p",pre_block);
    return pre_block;
}

sf_header *post_coales( sf_header * pre_block ,sf_header * post_block , int pre_block_size , int post_block_size){
    debug("post_coales : pre_block_size : %d post_block_size : %d",pre_block_size , post_block_size);
    debug("post_coales : pre_block_addr : %p post_block_addr : %p",pre_block , post_block);
    if(post_block->info.block_size * 16 != post_block_size){
        debug("pre coales : Inonsistent pre_block size : in header : %d and we are expecting : %d" ,
         (pre_block->info.block_size * 16) , pre_block_size);
    }
    free_from_free_list(post_block,post_block_size);
    int total_size = pre_block_size + post_block_size;
    sf_block_info info = {0,0,0,0,0};
    info.allocated = 0;
    info.prev_allocated = 1;
    info.two_zeroes = 0;
    info.block_size = total_size/16;
    info.requested_size = 0;
    pre_block->info = info;
    sf_block_info *last_pos = (void *)pre_block + (total_size - 8);
    *last_pos = info;
    return pre_block;
}


sf_header *search_free_block(int total){
    //int block_size = size - 8;
    //sf_header *head_in_sf_header_format = head;

    sf_free_list_node *node = sf_free_list_head.next;

    while( node !=  &sf_free_list_head){
        // if exact block size found
        if(node->size - 8 < total){
            node = node->next;
            debug("Block is smaller skipping : %d total : %d",(int)node->size - 8,total);
            continue;
        }
        debug("Big block size is : : %d",(int)node->size);
        if(node->size - 8 >= total){
            sf_header start_node = node->head;
            sf_header *next_node = start_node.links.next;
            if(next_node == &(node->head)){
                node = node->next;
                continue;
            }
        }
        debug("Block found");
        sf_header *start_node = &(node->head);
        sf_header *next_node = start_node->links.next;
        sf_header *next_to_first = next_node->links.next;

        sf_header *target_node = next_node;

        start_node->links.next = next_to_first; //target_node->links.next;
        next_to_first->links.prev = start_node;

        return target_node;

    }
    debug("No block found");
    return NULL;
}

void free_from_free_list(sf_header *block,int block_size){
    int usable_size = block_size - 8;
    sf_free_list_node *node = sf_free_list_head.next;
    debug("free_from_free_list : trying to get a free_list : for size : %d",block_size);
    while( node !=  &sf_free_list_head){
        // if exact block size found
        debug("node size is : %d",(int)node->size);
        if(node->size != usable_size + 8){
            node = node->next;
            continue;
        }

        sf_header *start_node = &(node->head);
        sf_header *internal_node = start_node->links.next;
        while(internal_node != start_node){
            debug("address being searched : %p",block);
            debug("address being    found : %p",internal_node);
            if(internal_node == block){
                sf_header *next = internal_node->links.next;
                sf_header *prev = internal_node->links.prev;
                next->links.prev = prev;
                prev->links.next = next;
                debug("Able to remove a particular block : of particular size : Big achivement");
                return;
            }else{
                internal_node = internal_node->links.next;
            }
        }
        debug("Could not find the address in the given size");
        node = node->next;
    }
    debug("Could not find a free list for the asked size");

}