#include <stdio.h>
#include <stdlib.h>
#include "alloc.h"

/**
    Implementation of memory allocation functions
**/


/**
    Allocates a block of size*n filled with 0's
    @param size size of type
    @param n number of blocks of size "size"
    @return Allocated poiner
**/
void* callocx(size_t size, size_t n){
    void* ptr = calloc(size,n);
    if(ptr==NULL){
        fprintf(stderr,"%s\n","Could not allocate memory");
        perror("Erro");
        exit(EXIT_FAILURE);
    }
    return(ptr);
}



void* reallocx(void* ptr,size_t size){
    void *aux = realloc(ptr,size);
    if(size==0){
        if(aux!=NULL){
            fprintf(stderr,"%s\n","Could not allocate memory");
            perror("Erro");
            exit(EXIT_FAILURE);
        }
    }
    else if(aux==NULL){
        fprintf(stderr,"%s\n","Could not allocate memory");
        perror("Erro");
        exit(EXIT_FAILURE);
    }
    return(aux);
}
