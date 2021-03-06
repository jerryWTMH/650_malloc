#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "my_malloc.h"

// void * global_base = NULL;
Blockmeta * firstFreeBlock = NULL;
Blockmeta * lastFreeBlock = NULL;
//Blockmeta * firstDataBlock = NULL;
size_t totalDataSize = 0;
size_t totalFreeSize = 0;



void insert_block(Blockmeta * blockPtr){
   if((firstFreeBlock == NULL) || (blockPtr < firstFreeBlock)){
        // block < firstFreeBlock just give a correct sequence
        blockPtr->prev = NULL;
        blockPtr->next = firstFreeBlock;
        firstFreeBlock = blockPtr;
        if(blockPtr->next == NULL || blockPtr == lastFreeBlock){        
            lastFreeBlock = blockPtr;
        } else{
            blockPtr->next->prev = blockPtr;
        }
    } else if(blockPtr > lastFreeBlock){
        lastFreeBlock->next = blockPtr;
        blockPtr->prev = lastFreeBlock;
        blockPtr->next = NULL;
        lastFreeBlock = blockPtr;
    } 
    else{
        // block should be put in the middle of the list
        Blockmeta * curr;
        bool overLastOne = true;
        for(curr = firstFreeBlock; curr->next != NULL; curr = curr->next){
            if(blockPtr <= curr->next){
                overLastOne = false;
                break;
            }
        }
        blockPtr->prev = curr;
        if(overLastOne){
            blockPtr->next = NULL;
            curr-> next = blockPtr;
        } else{
            blockPtr->next = curr->next;
            curr->next = blockPtr;
            blockPtr->next->prev = blockPtr;
        }

    }
    //  if((firstFreeBlock == NULL) || (blockPtr < firstFreeBlock)){
    //     // block < firstFreeBlock just give a correct sequence
    //     blockPtr->prev = NULL;
    //     blockPtr->next = firstFreeBlock;
    //     firstFreeBlock = blockPtr;
    //     if(blockPtr->next == NULL || blockPtr == lastFreeBlock){        
    //         lastFreeBlock = blockPtr;
    //     } else{
    //         blockPtr->next->prev = blockPtr;
    //     }
    // } else if(blockPtr > lastFreeBlock){
    //     lastFreeBlock->next = blockPtr;
    //     blockPtr->prev = lastFreeBlock;
    //     blockPtr->next = NULL;
    //     lastFreeBlock = blockPtr;
    // } 
    // else{
    //     // block should be put in the middle of the list
    //     Blockmeta * curr = firstFreeBlock;
    //     bool overLastOne = true;
    //     for(Blockmeta* curr = firstFreeBlock; curr->next != NULL; curr = curr->next){
    //         if(blockPtr < curr->next){
    //             overLastOne = false;
    //             break;
    //         }
    //     }
    //     blockPtr->prev = curr;
    //     if(overLastOne){
    //         blockPtr->next = NULL;
    //         curr-> next = blockPtr;
    //     } else{
    //         blockPtr->next = curr->next;
    //         curr->next = blockPtr;
    //         blockPtr->next->prev = blockPtr;
    //     }
    // }
}

void delete_block(Blockmeta * blockPtr){
    if(blockPtr == firstFreeBlock){
        if(lastFreeBlock == firstFreeBlock){
            lastFreeBlock = firstFreeBlock = NULL;
        } else{
            firstFreeBlock = blockPtr->next;
            firstFreeBlock->prev = NULL;
        }
    } else{
        if(blockPtr != lastFreeBlock){
            blockPtr ->prev->next= blockPtr -> next;
            blockPtr ->next ->prev = blockPtr -> prev;
        } else{
            lastFreeBlock = blockPtr -> prev;
            lastFreeBlock -> next = NULL;
        }
    }
}

Blockmeta * get_sliced_block(Blockmeta * blockPtr, size_t size){
    // return a splitted block.
        Blockmeta * slicedBlock;
        slicedBlock = (Blockmeta *)((char *)blockPtr + META_SIZE + size);
        slicedBlock -> size = blockPtr->size;
        slicedBlock -> size -= (size + META_SIZE);
        slicedBlock -> next = NULL;
        slicedBlock -> prev = NULL;
        // useless
        Blockmeta * temp;
        temp = slicedBlock;
        temp->size = slicedBlock->size;
        temp -> next = NULL;
        temp -> prev = NULL;
        return temp;
}

// void * use_existing_block(size_t size, Blockmeta * blockPtr){
//     if(blockPtr->size > size + sizeof(Blockmeta)){
//         // slicedBlock is the memory space left after allocating.
//         Blockmeta * slicedBlock = get_sliced_block(blockPtr, size);
//         // wanna use block p, so we need to remove it in the list
//         delete_block(blockPtr);
//         // wanna add the slicedBlock into the list
//         insert_block(slicedBlock);
//         blockPtr->size = size;
//         totalFreeSize -= (size + sizeof(Blockmeta));
//     } else{
//         delete_block(blockPtr);
//         totalFreeSize -= (sizeof(Blockmeta) + blockPtr->size); 
//     }
//     blockPtr->isfree = false;
//     blockPtr->prev = NULL;
//     blockPtr->next = NULL;
//     return (char *)blockPtr + sizeof(Blockmeta);

// }

void * allocate_block(size_t size){
    
      
    Blockmeta * newBlock = sbrk(META_SIZE+ size);
    // if(firstDataBlock == NULL){
    //     firstDataBlock = newBlock;
    // }
    newBlock -> size = size;    
    newBlock -> prev = NULL;
    newBlock -> next = NULL;
    // printf("newBlock -> size:%lu \n", newBlock->size);
    totalDataSize = totalDataSize + META_SIZE + size;  
    return (char*)newBlock + META_SIZE;
}

void check_merge(Blockmeta * p){
    //if the previous space is also a free space
    bool left_part_merge = false;
    bool right_part_merge = false;
    if(p->prev){
        if((char *)p->prev + p->prev->size + META_SIZE == (char *)p){
            left_part_merge = true;
        }
    }
    if(p->next){
        if((char *)p + p->size + META_SIZE == (char *)p->next){
            right_part_merge = true;
        }
    }
    if(left_part_merge && right_part_merge){
       p->prev->size += META_SIZE + p->size;
       delete_block(p);
       p->size += META_SIZE + p->next->size;       
       delete_block(p->next);
    } else if(left_part_merge){
        p->prev->size += META_SIZE + p->size;
        delete_block(p);
    } else if(right_part_merge){
       p->size += META_SIZE + p->next->size;
       delete_block(p->next);
    }
}

void * ff_malloc(size_t size){
    if(firstFreeBlock != NULL){ 
        Blockmeta * ptr = firstFreeBlock;
        for(Blockmeta*ptr = firstFreeBlock; ptr != NULL; ptr = ptr->next){
            if(ptr->size >= size ){
                if(ptr->size > size + META_SIZE){
                    // slicedBlock is the memory space left after allocating.
                    Blockmeta * slicedBlock = get_sliced_block(ptr, size);
                    // wanna use block p, so we need to remove it in the list
                    delete_block(ptr);
                    // wanna add the slicedBlock into the list
                    insert_block(slicedBlock);
                    ptr->size = size;
                    totalFreeSize -= (size + META_SIZE);
                } else{
                    delete_block(ptr);
                    totalFreeSize -= (META_SIZE + ptr->size); 
                }
                ptr->prev = NULL;
                ptr->next = NULL;
                return (char *)ptr + META_SIZE;
            }
        }
    }
    return allocate_block(size);
}

void ff_free(void * ptr){
    Blockmeta * realPtr;
    realPtr = (Blockmeta *)((char *)ptr - sizeof(Blockmeta));
    realPtr->isfree = true;
    totalFreeSize += realPtr->size + sizeof(Blockmeta);
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
            if(p->size >= size){
                size_t diff = p->size - size;
                if(diff < min){
                    min = diff;
                    min_ptr = p;
                } else if(diff == 0){
                    min_ptr = p;
                    break;
                }
            }
            p = p->next;
        }
        if(min_ptr == NULL){
            // cannot find a suitable free block!
            return allocate_block(size);
        } else{
            // using min_ptr block
            if(min_ptr->size > size + META_SIZE){
                // slicedBlock is the memory space left after allocating.
                Blockmeta * slicedBlock = get_sliced_block(min_ptr, size);
                // wanna use block p, so we need to remove it in the list
                delete_block(min_ptr);
                // wanna add the slicedBlock into the list
                insert_block(slicedBlock);
                min_ptr->size = size;
                totalFreeSize -= (size + META_SIZE);
            } else{
                delete_block(min_ptr);
                totalFreeSize -= (META_SIZE + min_ptr->size); 
            }
            min_ptr->prev = NULL;
            min_ptr->next = NULL;
            return (char *)min_ptr + META_SIZE;
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