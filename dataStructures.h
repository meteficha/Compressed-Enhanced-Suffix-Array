#ifndef DATASTRUCTURES_H_INCLUDED
#define DATASTRUCTURES_H_INCLUDED

#include "types.h"

#define VECTOR_DEFAULT_SIZE 1<<10

typedef struct vector{
    word* elements; /**words of the vector**/
    word limit; /** upper bound limit of the container**/
    word size; /**size in bits**/
}vector;


vector* createVector(word n);
void addVector(vector* V, word w);
void resizeVector(vector* V,word w);
void freeVector(vector* V);

#endif // DATASTRUCTURES_H_INCLUDED
