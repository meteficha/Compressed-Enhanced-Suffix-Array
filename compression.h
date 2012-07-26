#ifndef COMPRESSION_H_INCLUDED
#define COMPRESSION_H_INCLUDED

#include "dataStructures.h"

void diferentialCoding(vector* V);
void unaryCoding(vector* V,bitVector* B);
void differentialCoding(vector* V);
bitVector* psiCoding(vector* V);

#endif // COMPRESSION_H_INCLUDED
