#include <string.h>
#include <math.h>
#include "bit.h"
#include "compression.h"
#include "dataStructures.h"

/**differentialCoding, increasing sequence of integers is assumed**/
void differentialCoding(vector* V){
    word i;
    for(i=1;i<V->size;i++){
        V->elements[i] -= V->elements[i-1];
    }
}

bitVector* psiCoding(vector* V){
    differentialCoding(V);
    bitVector* B = createBitVector(V->size*2);
    unaryCoding(V,B);
    /**Resize bitVector**/
    /**Free Vector**/
    freeVector(V);
    return(B);
}

void unaryCoding(vector* V,bitVector* B){
    word i;
    word index = 0;

    for(i=0;i<V->size;i++){
        index += V->elements[i];
        setBit(B,index);
    }
}

