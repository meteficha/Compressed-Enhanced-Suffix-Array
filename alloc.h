#ifndef ALLOC_H_INCLUDED
#define ALLOC_H_INCLUDED
#include <stdlib.h>

/**
    Memory allocation definitions
**/

void* callocx(size_t size,size_t n);
void* reallocx(void* ptr,size_t size);

#endif // ALLOC_H_INCLUDED
