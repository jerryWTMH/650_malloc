#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "my_malloc.h"

// void * global_base = NULL;
Blockmeta * first_free_block = NULL;
Blockmeta * last_free_block = NULL;

size_t data_size = 0;
size_t free_size = 0;

void add_block(Blockmeta * block){
    if((first_free_block == NULL) || (block < first_free_block)){
        // block < first_free_block just give a correct sequence
        block->prev = NULL;
        block->next = first_free_block;
        if(first_free_block == NULL){            
            last_free_block = block;
        } else{
            first_free_block -> prev = block;
        }
        first_free_block = block;
    } else{
        // block should be put in the middle of the list
        Blockmeta * curr = first_free_block;
        while((curr->next != NULL) && (block > curr->next)){
            curr = curr -> next;
        }
        block->prev = curr;
        block->next = curr -> next;
        curr-> next = block;
        if(curr->next == NULL){
            // if there is originally only one element in the list.
            last_free_block = block;
        } else{
            curr->next->prev = block;
        }

    }
}

void remove_block(Blockmeta * p){
    // adjusting the pointer in the LinkedList
    if((first_free_block == last_free_block) && (first_free_block == p)){
        // p is the only one left in the free block
        last_free_block = first_free_block = NULL;
    } else if(first_free_block == p){
        // p is the first free block
        first_free_block = p -> next;
        first_free_block -> prev = NULL;
    } else if(last_free_block ==p){
        // p is the last free block
        last_free_block = p -> prev;
        last_free_block -> next = NULL;
    } else{
        // p is the middle free block
        p -> prev -> next= p -> next;
        p -> next -> prev = p -> prev;
    }
}

void * reuse_block(size_t size, Blockmeta * p){
    if(p -> size > size + sizeof(Blockmeta)){
        // splitted_block is the memory space left after allocating.
        Blockmeta * splitted_block;
        splitted_block = (Blockmeta *)((char *)p + sizeof(Blockmeta) + size);
        splitted_block -> size = p -> size - size - sizeof(Blockmeta);
        splitted_block -> isfree = 1;
        splitted_block -> next = NULL;
        splitted_block -> prev = NULL;

        // wanna use block p, so we need to remove it in the list
        remove_block(p);
        // wanna add the splitted_block into the list
        add_block(splitted_block);
        p->size = size;
        free_size -= (size + sizeof(Blockmeta));
    } else{
        remove_block(p);
        free_size -= (p->size + sizeof(Blockmeta)); ///////////
    }
    p->isfree = 0;
    p->prev = NULL;
    p->next = NULL;
    return (char *)p + sizeof(Blockmeta);

}

void * allocate_block(size_t size){
    data_size += sizeof(Blockmeta) + size;
    Blockmeta * new_block = sbrk(data_size);
    if(first_free_block == NULL){
        first_free_block = new_block;
    }
    new_block -> size = size;    
    new_block -> prev = NULL;
    new_block -> next = NULL;
    new_block -> isfree = 0;
    printf("new_block -> size:%lu \n", new_block->size);
    
    return (char*) new_block + sizeof(Blockmeta);
}



void * ff_malloc(size_t size){
    if(first_free_block != NULL){ 
        Blockmeta * p = first_free_block;
        while(p != NULL){
            if(p -> size >= size){
                return reuse_block(size, p);
            }
            p = p->next;
        }
    }
    return allocate_block(size);
}

void ff_free(void * ptr){
    Blockmeta * p;
    p = (Blockmeta *)((char *)ptr - sizeof(Blockmeta));
    p->isfree = 1;
    add_block(p);

    if((p->prev != NULL) && ((char *)p->prev + p->prev->size + sizeof(Blockmeta) == (char *)p)){
        p->prev->size += sizeof(Blockmeta) + p->size;
        remove_block(p);
    }

    if((p->next != NULL) && ((char *)p + p->size + sizeof(Blockmeta) == (char *)p->next)){
        p->size += sizeof(Blockmeta) + p->next->size;
        remove_block(p->next);
    }
}

void * bf_malloc(size_t size){
 void * something = NULL;
 return something;
}

void bf_free(void * ptr){
    
}
