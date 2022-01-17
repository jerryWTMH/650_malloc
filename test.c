#include <stdlib.h>
#include <stdio.h>
#include "my_malloc.h"

int main(int argc, char* argv[]){
    int length = 5;
    Blockmeta * list[length];
    list[0] = ff_malloc(30);
    list[1] = ff_malloc(70);
    list[2] = ff_malloc(44);
    list[3] = ff_malloc(1000);
    list[4] = ff_malloc(77);
    ff_free(list[2]);
    for(int i = 0; i < length ; i++){
        while(list[i] != NULL){
            Blockmeta * temp = (Blockmeta *)((char *)list[i] - sizeof(Blockmeta));
            printf("list [%d]: %lu \n", i, temp->size);
            list[i] = list[i]->next;
        }
    }

    
    printf("Ending! \n");
    return EXIT_SUCCESS;
}