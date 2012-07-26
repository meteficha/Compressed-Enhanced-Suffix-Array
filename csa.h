#ifndef CSA_H_INCLUDED
#define CSA_H_INCLUDED

#include "types.h"
#include "text.h"
#include "bit.h"

#define SA_SAMPLE_RATE 20

/**the psi function representation for walking through the compressed SA**/

typedef struct psiStructure{
    bitVector* psiQuotient[ALPHABET_SIZE]; /**The quotient part**/
    bitVector* psiRemainder[ALPHABET_SIZE]; /**The remainder part**/
    word psiRemainderBits[ALPHABET_SIZE]; /**the number of bits in the quotient**/
    word psiQuotientBits[ALPHABET_SIZE]; /**the number of bits in the quotient**/
    word lastCodedPsiQuotient[ALPHABET_SIZE]; /**contains the last Coded psi value**/
    word lastQuotientBit[ALPHABET_SIZE]; /**contains the position of the last set bit**/
    word currentWordSize; /** The size of word for each psi entry**/
}psiStructure;


typedef struct saStructure{
    word* array;
    word sampleRate;
    word words;
}saStructure;

typedef struct lcpStructure{
    bitVector* lcpQuotient;
    word lastCodedLcpQuotient;
    word lastQuotientBit;
}lcpStructure;





/**definition for the compressed enhanced suffix array**/
typedef struct cesa{
    saStructure* sa; /**the compact representation of the SA table**/
    lcpStructure* lcp; /**the compact representation of the lcp table**/
    psiStructure* psi;
    word c[ALPHABET_SIZE+1]; /**array that keeps the number of characters lexicographically smaller than c[i]**/
    text* T; /**Associated text of the Compressed enhanced suffix array**/
    text* P; /**Pattern file for searches**/
    FILE* Index; /**File in which the index will be saved**/
    word n; /**size of the cesa*/
}cesa;




word accessPsi(cesa*  CSA, word i);
cesa* createCSA(text* T);
cesa* computeCSA(text* T);
word rankBsearch(cesa* CSA,word rankNextSuffix,word ch);
void computeRankAmongShortSuffixes(cesa* CSA,word currentBlockSize);
void computeRankAmongLongSuffixes(cesa* CSA, word currentBlockSize);
void computeNewPsi(cesa* CSA,word currentBlockSize,word start);
void baseCase(cesa* CSA,word currentBlockSize);
word accessPsi(cesa* CSA,word i);
void setPsiValue(cesa* CSA,word i ,word value);
void configurePsi(cesa* CSA);
void preprocessQuotientRank(cesa* CSA);
void sampleSA(cesa* CSA);
word accessSA(cesa* CSA,word i);
void freeCSA(cesa* CSA);

void createLcp(cesa* CSA);
void setLcpValue(cesa* CSA, word saValue,word lcpValue);
word accessLcp(cesa* CSA, word i);
void computeLcp(cesa* CSA);
word lcpCompare(cesa* CSA,word i,word k,word h);
void saveIndex(cesa* CSA,char* fileName);
cesa* loadIndex(char* fileName);
#endif // CSA_H_INCLUDED
