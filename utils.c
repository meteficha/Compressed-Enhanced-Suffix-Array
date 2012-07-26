#include <math.h>
#include "utils.h"

double log2 (double n){


        return(log(n)/log(2.0));
}


int compareWords(const void* a, const void* b){
    word *c,*d;
    c = a;
    d = b;
    if(*c<*d) return(-1);
    return (1);
}

int compareSuffixes(const void* a, const void* b){
    suffixStructure *c,*d;
    c = a;
    d = b;
    if(c->P < d->P){
        return(-1);
    }
    else if(c->P > d->P){
        return(1);
    }
    else{
        if(c->Q < d->Q){
            return(-1);
        }
        return (1);
    }
}
