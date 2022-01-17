#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "my_malloc.h"

// void * global_base = NULL;
Blockmeta * firstFreeBlock = NULL;
Blockmeta * lastFreeBlock = NULL;
Blockmeta * firstDataBlock = NULL;
size_t totalDataSize = 0;
size_t totalFreeSize = 0;



void insert_block(Blockmeta * blockPtr){
    if((firstFreeBlock == NULL) || (blockPtr < firstFreeBlock)){
        // block < firstFreeBlock just give a correct sequence
        blockPtr->prev = NULL;
        blockPtr->next = firstFreeBlock;
        if(blockPtr->next == NULL){        
            lastFreeBlock = blockPtr;
        } else{
            blockPtr->next->prev = blockPtr;
        }
        firstFreeBlock = blockPtr;
    } else{
        // block should be put in the middle of the list
        Blockmeta * curr = firstFreeBlock;
        while((curr->next != NULL) && (blockPtr > curr->next)){
            curr = curr->next;
        }
        blockPtr->prev = curr;
        blockPtr->next = curr->next;
        curr->next = blockPtr;
        if(blockPtr->next == NULL){
            // if there is originally only one element in the list.
            lastFreeBlock = blockPtr;
        } else{
            blockPtr->next->prev = blockPtr;
        }

    }
}

void delete_block(Blockmeta * blockPtr){
    // adjusting the pointer in the LinkedList
    if((lastFreeBlock == firstFreeBlock) && (lastFreeBlock == blockPtr)){
        // p is the only one left in the free block
        lastFreeBlock = firstFreeBlock = NULL;
    } else if(firstFreeBlock == blockPtr){
        // p is the first free block
        firstFreeBlock = blockPtr->next;
        firstFreeBlock->prev = NULL;
    } else if(lastFreeBlock ==blockPtr){
        // p is the last free block
        lastFreeBlock = blockPtr -> prev;
        lastFreeBlock -> next = NULL;
    } else{
        // p is the middle free block
        blockPtr ->prev->next= blockPtr -> next;
        blockPtr ->next ->prev = blockPtr -> prev;
    }
}

Blockmeta * get_sliced_block(Blockmeta * blockPtr, size_t size){
    // return a splitted block.
        Blockmeta * slicedBlock;
        slicedBlock = (Blockmeta *)((char *)blockPtr + sizeof(Blockmeta) + size);
        slicedBlock -> size = blockPtr->size - size - sizeof(Blockmeta);
        slicedBlock -> isfree = 1;
        slicedBlock -> next = NULL;
        slicedBlock -> prev = NULL;
        return slicedBlock;
}

void * use_existing_block(size_t size, Blockmeta * blockPtr){
    if(blockPtr->size > size + sizeof(Blockmeta)){
        // slicedBlock is the memory space left after allocating.
        Blockmeta * slicedBlock = get_sliced_block(blockPtr, size);
        // wanna use block p, so we need to remove it in the list
        delete_block(blockPtr);
        // wanna add the slicedBlock into the list
        insert_block(slicedBlock);
        blockPtr->size = size;
        totalFreeSize -= (size + sizeof(Blockmeta));
    } else{
        delete_block(blockPtr);
        totalFreeSize -= (sizeof(Blockmeta) + blockPtr->size); 
    }
    blockPtr->isfree = 0;
    blockPtr->prev = NULL;
    blockPtr->next = NULL;
    return (char *)blockPtr + sizeof(Blockmeta);

}

void * allocate_block(size_t size){
    Blockmeta * newBlock = sbrk(sizeof(Blockmeta)+ size);
    if(firstDataBlock == NULL){
        firstDataBlock = newBlock;
    }
    newBlock -> size = size;    
    newBlock -> prev = NULL;
    newBlock -> next = NULL;
    newBlock -> isfree = 0;
    // printf("newBlock -> size:%lu \n", newBlock->size);
    totalDataSize = totalDataSize + sizeof(Blockmeta) + size;    
    return (char*)newBlock + sizeof(Blockmeta);
}

void check_merge(Blockmeta * p){
    //if the previous space is also a free space
    if(p->prev != NULL){
        if((char *)p->prev + p->prev->size + sizeof(Blockmeta) == (char *)p){
            p->prev->size += sizeof(Blockmeta) + p->size;
        delete_block(p);
        }
    }
    //if the latter space is also a free space
    if(p->next != NULL){
        if((char *)p + p->size + sizeof(Blockmeta) == (char *)p->next){
            p->size += sizeof(Blockmeta) + p->next->size;
            delete_block(p->next);
        }
    }
}

void * ff_malloc(size_t size){
    if(firstFreeBlock != NULL){ 
        Blockmeta * ptr = firstFreeBlock;
        while(ptr != NULL){
            if(size < ptr -> size){
                return use_existing_block(size, ptr);
            }
            ptr = ptr->next;
        }
    }
    return allocate_block(size);
}

void ff_free(void * ptr){
    Blockmeta * realPtr;
    realPtr = (Blockmeta *)((char *)ptr - sizeof(Blockmeta));
    realPtr->isfree = 1;
    // insert the block into the free block list.
    insert_block(realPtr);
    // check whether there would be some adjacent free blocks near the block that ptr points to.
    check_merge(realPtr);
}

void * bf_malloc(size_t size){
        Blockmeta * p = firstFreeBlock;
        size_t min = __SIZE_MAX__;
        Blockmeta * min_ptr = NULL;
        while(p != NULL){
            if(size < p->size){
                size_t diff = p->size - size;
                if(diff < min){
                    min = diff;
                    min_ptr = p;
                }
            }
            p = p->next;
        }
        if(min_ptr == NULL){
            // cannot find a suitable free block!
            return allocate_block(size);
        } else{
            // using min_ptr block
            return use_existing_block(size, min_ptr);
        }
        
    
}

void bf_free(void * ptr){
    // The bf_free is the same as the ff_free
    ff_free(ptr);
}

unsigned long get_data_segment_size() {
  return totalDataSize;
}

unsigned long get_data_segment_free_space_size() {
  return totalFreeSize;
}

