#include <stdio.h>
#include <stdlib.h>

struct blockmeta{
    size_t size;
    int isfree;
    struct blockmeta * next;
    struct blockmeta * prev;
};
typedef struct blockmeta Blockmeta;

void * ff_malloc(size_t size);
void ff_free(void * ptr);
void * bf_malloc(size_t size);
void bf_free(void * ptr);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();

void printFreeList();
void * reuse_block(size_t size, Blockmeta * p);
void * allocate_block(size_t size);
void add_block(Blockmeta * p);
void remove_block(Blockmeta * p);
