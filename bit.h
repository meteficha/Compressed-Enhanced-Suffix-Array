#ifndef BIT_H_INCLUDED
#define BIT_H_INCLUDED

#include "types.h"


#define RANK_FACTOR 12 /**determine the amount of words in each superblock**/


/**
    Data structures and declarations for manipulating bit-vectors with support
    to rank and selection operations.
**/


typedef struct bitVector{
    word* v; /**bit-vector**/
    word* rankLookUp; /**sampled table for rank look-up**/
    word size; /**number of bits**/
    word words; /**number of words**/
    word rankWords; /**number of rank words**/
}bitVector;


/***** INITIALIZATION OF LOOK-UP TABLES *****/

/**
    initializes the MaskAcess array
**/
void initMaskAccess(void);
void initPopCount(void);
void initRangeAccess(void);
void initAllBitTables(void);

/***** MANIPULATION OF THE BIT-VECTOR DATA STRUCTURE *****/

bitVector* createBitVector(word n);
void freeBitVector(bitVector* B);
void preprocessRank(bitVector* B);
void resizeBitVector(bitVector* B, word size);
void resetBitVector(bitVector* B);

/***** OPERATIONS IN THE BIT-VECTOR*****/

word rank1(bitVector* B,word n);
word rank0(bitVector* B,word n);
word select1(bitVector* B, word i);
word select0(bitVector* B, word i);
word retrieveBit(word w,word i);
word popCountWord(word w);
word sequentialPopCount(bitVector* B,word start,word end);
void setBitsInBitVector(bitVector* B,word w,word nBits,word start);
void setBit(bitVector* B,word i);
word readBitFromBitVector(bitVector* B,word i);
word extractBitsFromBitVector(bitVector* B,word start, word end);



/****** UTILITIES FUNCTION *******/
word retrieveNumBits(word n);
word concatenateWords(word a, word b,word nBitsa);
word selectBitsFromWord(word a,int i,int j);

void saveBitVector(bitVector* B,FILE* ptr);
bitVector* loadBitVector(FILE* ptr);
#endif // BIT_H_INCLUDED
