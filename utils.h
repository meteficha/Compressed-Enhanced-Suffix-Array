#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "types.h"
#define max(a,b,c) (c) = (a) > (b) ? (a) : (b);
#define min(a,b,c) (c) = (a) < (b) ? (a) : (b);

double log2 (double n);
int compareSuffixes(const void* a, const void* b);
int compareWords(const void* a, const void* b);
typedef struct suffixStructure{
    word index;
    word P;
    word Q;
}suffixStructure;
#endif // UTILS_H_INCLUDED
