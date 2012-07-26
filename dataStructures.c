#include <stdlib.h>
#include "dataStructures.h"
#include "alloc.h"


/**Create the vector**/
vector* createVector(word n){
    vector* V = callocx(sizeof(vector),1);
    /**Creates vector with the default size**/
    if(n==0){
        V->elements = callocx(sizeof(word),VECTOR_DEFAULT_SIZE);
        V->limit = VECTOR_DEFAULT_SIZE;
   }
   /**Creates vector with the specified size**/
   else{
        V->elements = callocx(sizeof(word),n);
        V->limit = n;
   }
   V->size = 0;
   return(V);
}


/**Add a word to the vector**/

void addVector(vector* V, word w){
    if(V->size == V->limit){
        V->limit *=2;
        reallocx(V->elements,sizeof(word)*V->limit);
    }
    V->elements[V->size++]=w;
}

void freeVector(vector* V){
    free(V->elements);
    free(V);
}
