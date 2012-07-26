#include <math.h>
#include <string.h>
#include "utils.h"
#include "alloc.h"
#include "bit.h"
/**
    Implementation of bit operations
**/


/**
    Masks for isolation of the ith-bit in a word
**/
static word maskAccess[WORD_SIZE];
/**
    Masks for isolation of a specif bit-range in a word
**/
static word maskRangeAccess[WORD_SIZE][WORD_SIZE];
static word bitPopCount[8][256];
static word popCount[256];
static word one = 1;
static word zero = 0;

/***** INITIALIZATION OF LOOK-UP TABLES *****/


/**
    Initializes the maskAccess array. Most Significant bit
    occupies position 0.
**/
void initMaskAccess(void){
    word i;
    for(i=0;i<WORD_SIZE;i++){
        maskAccess[i] = one << (word)(WORD_SIZE - i -1);
    }
}

void initRangeAccess(void){
    word i,j;
    for(i=0;i<WORD_SIZE;i++){
        maskRangeAccess[i][i] =  maskAccess[i];
        for(j=i+1;j<WORD_SIZE;j++){
            maskRangeAccess[i][j] = maskRangeAccess[i][j-1] | (maskAccess[j]);
        }
    }
}

/**
    Initializes the popCount and the bitPopCount arrays
**/
void initPopCount(void){
    word i,j,c;
    for(i=0;i<256;i++){
        c=0;
        for(j=0;j<8;j++){
            if(retrieveBit(i,WORD_SIZE-8+j)==1){
                bitPopCount[j][i] = ++c;
            }
            else{
                bitPopCount[j][i] = c;
            }
        }
    }
    for(j=0;j<256;j++){
        popCount[j] = bitPopCount[7][j];
    }
}

void initAllBitTables(void){
    initMaskAccess();
    initPopCount();
    initRangeAccess();
}

/***** MANIPULATION OF THE BIT-VECTOR DATA STRUCTURE *****/

bitVector* createBitVector(word n){
    bitVector* B = callocx(1,sizeof(bitVector));
    word numWords = ceil((double)n/WORD_SIZE);
    if(n==0){
        B->v=NULL;
    }
    else{
        B->v = callocx(numWords,sizeof(word));
    }
    B->size = n;
    B->rankLookUp = NULL;
    B->words = numWords;
    B->rankWords = 0;
    return(B);
}


void freeBitVector(bitVector* B){
    if(B->v != NULL){
        free(B->v);
        if(B->rankLookUp!=NULL){
            free(B->rankLookUp);
        }
    }
    free(B);
}

/**
    Preprocess the bit-vector to compute the look-up table
    of rank.
**/
void preprocessRank(bitVector* B){
    word i;
    word totalBits;
    word c;
    word start,end;
    word words;
    if(B->v!=NULL){
        words = B->rankWords;
        B->rankWords =  ceil((double)B->size/(RANK_FACTOR*WORD_SIZE));
        if(words != B->rankWords)
            B->rankLookUp = reallocx(B->rankLookUp,B->rankWords * sizeof(word));

        totalBits = B->size;
        start = 0;
        B->rankLookUp[0] = 0;
        end =  WORD_SIZE*RANK_FACTOR -1 < totalBits-1 ?
            WORD_SIZE*RANK_FACTOR -1 : totalBits-1 ;
        c=0;
        i=1;
        while(i< B->rankWords){
            c+=sequentialPopCount(B,start,end);
            totalBits -= (end-start+1);
            start = end+1;
            end =  WORD_SIZE*RANK_FACTOR -1 < totalBits-1 ?
                start+WORD_SIZE*RANK_FACTOR -1 : start+totalBits-1 ;
            B->rankLookUp[i]=c;
            i++;
        }
    }
}


void resizeBitVector(bitVector* B, word size){
    B->size = size;
    B->words = ceil((double)size/WORD_SIZE);
    B->v = reallocx(B->v,sizeof(word)*B->words);
}



/***** OPERATIONS IN  THE BIT-VECTOR*****/


/**
    Returns the numbers of 1's of the Bit-vector's preffix of length
    n. To do this, we do a look-up in the rank table and
    a sequential population count.
**/

word rank1(bitVector* B,word n){
    word superBlock = n/(WORD_SIZE*RANK_FACTOR);
    word start = superBlock*WORD_SIZE*RANK_FACTOR;
    word end = n;
    return(B->rankLookUp[superBlock] + sequentialPopCount(B,start,end));
}

word rank0(bitVector* B,word n){
    return(n+1-rank1(B,n));
}

/**
    Returns the position of the ith 1 bit of the Bit-vector.
**/

word select1(bitVector* B, word i){
    word mid,l,r,c,temp;
    word offset;
    word wStart;
    word popC;
    word index=0;
    l=0;
    r=B->rankWords-1;
    /**Binary Search for the superblock**/
    while(l<r){
        mid = (l+r)/2;
        if(B->rankLookUp[mid] < i){
            l = mid+1;
            index = mid;
        }
        else{
            r=mid-1;
        }
    }
    if(l==r){
        if(B->rankLookUp[l]<i){
            index = l;
        }
        else if(B->rankLookUp[l]==i){
            index = l;
            if(index>0) index--;
        }
    }
    offset = index*RANK_FACTOR*WORD_SIZE;
    /**Sequential search using popCount**/
    c = i - B->rankLookUp[index];
    wStart = index*RANK_FACTOR;
    popC =  popCountWord(B->v[wStart]);
    while(c > popC){
        c -= popC;
        wStart++;
        popC =  popCountWord(B->v[wStart]);
        offset+= WORD_SIZE;
    }
    /**Sequential search using bitwise**/
    for(temp = 0;c!=0;temp++){
        if(retrieveBit(B->v[wStart],temp)==1){
            c--;
        }
    }
    offset+=temp-1;
    return(offset);
}

/**
    Returns the position of the ith 0 bit of the Bit-vector.
**/


word select0(bitVector* B, word i){
    word mid,l,r,c,temp;
    word offset;
    word wStart;
    word popC;
    word index = 0;
    l=0;
    r=B->rankWords-1;
    /**Binary Search for the superblock**/
    while(l<r){
        mid = (l+r)/2;
        if(mid*WORD_SIZE*RANK_FACTOR - B->rankLookUp[mid] < i){
            l = mid+1;
            index = mid;
        }
        else{
            r=mid-1;
        }
    }

   if(l==r){
        if(l*WORD_SIZE*RANK_FACTOR-B->rankLookUp[l]<i){
            index = l;
        }
        else if(l*WORD_SIZE*RANK_FACTOR - B->rankLookUp[l]==i){
            index = l;
            if(index>0) index--;
        }
    }

    offset = index*RANK_FACTOR*WORD_SIZE;
    /**Sequential search using popCount**/
    c = i - (index*WORD_SIZE*RANK_FACTOR  - B->rankLookUp[index]);
    wStart = index*RANK_FACTOR;
    popC =  WORD_SIZE - popCountWord(B->v[wStart]);
    while(c > popC){
        c -= popC;
        wStart++;
        popC = WORD_SIZE- popCountWord(B->v[wStart]);
        offset+= WORD_SIZE;
    }
    /**Sequential search using bitwise**/
    for(temp = 0;c!=0;temp++){
        if(retrieveBit(B->v[wStart],temp)==0){
            c--;
        }
    }
    offset+=temp-1;
    return(offset);
}

void setBit(bitVector* B,word i){
    word startW;
    startW =  i >> WORD_SHIFT;
    i = i & WORD_MOD;
    B->v[startW] |= maskAccess[i];
}



void setBitsInBitVector(bitVector* B,word w,word nBits, word start){
    word startW,sl,sr;
    startW = start >> WORD_SHIFT;
    start = start & WORD_MOD;
    if(WORD_SIZE-start  >= nBits){
        sl = WORD_SIZE-start-nBits;
        B->v[startW] &= ~(maskRangeAccess[start][start+nBits-1]);
        B->v[startW] |=  (w<<sl);
    }
    else{
        sr = nBits - WORD_SIZE + start;
        B->v[startW] &= (~maskRangeAccess[start][WORD_SIZE-1]);
        B->v[startW] |=  (w>>sr);
        B->v[startW+1] &= (~maskRangeAccess[0][(start+nBits-1)%WORD_SIZE]);
        B->v[startW+1] |=  (w << (WORD_SIZE - sr));
    }
}


/**
    Retrieve the ith-bit from a word.
    Least significant bit start from position zero.
**/
word retrieveBit(word w,word i){
    return( (w & maskAccess[i]) == zero ? zero : one);
}


/**
    Popcounts one word. 4 memory access if is 32-bit word, 8 access otherwise.
**/

word popCountWord(word w){
#ifdef ARCH_32
    return(popCount[w & 0xff] + popCount[(w >> 8) & 0xff] +
           popCount[(w >> 16) & 0xff] + popCount[(w >> 24) & 0xff]);
#else
    return(popCount[w & 0xff] +
           popCount[(w >> (word)8) & 0xff] +
           popCount[(w >> (word)16) & 0xff] +
           popCount[(w >> (word)24) & 0xff] +
           popCount[(w >> (word)32) & 0xff] +
           popCount[(w >> (word)40) & 0xff] +
           popCount[(w >> (word)48) & 0xff] +
           popCount[(w >> (word)56) & 0xff]);
#endif
}


/**
    Does a population count in the interval [start,end] of the bitVector.
    start must be a word boundarie.
**/

word sequentialPopCount(bitVector* B,word start,word end){
    word i,k,j;
    word c=0;
    /**For each word do a standard popCount**/
    for(i= (start >> WORD_SHIFT);i< (end >> WORD_SHIFT);i++){
        c+=popCountWord(B->v[i]);
    }

    /**Now we do popCounts until the last byte, which we look
    up in bitPopCount**/
    for(j=WORD_SIZE/8-1,k = end & WORD_MOD;k>7;j--,k-=8){
        c+=popCount[(B->v[i]>>(j<<3)) & 0xff];
    }
    c+=bitPopCount[k][(B->v[i]>>(j<<3)) & 0xff];
    return(c);
}


word readBitFromBitVector(bitVector* B,word i){
    word w = i>>WORD_SHIFT;
    return(retrieveBit(B->v[w],i & WORD_MOD));
}

/**Extract few bits (less than word size) of a bitvector return the word**/
word extractBitsFromBitVector(bitVector* B,word start, word end){
    word extracted;
    word h,l;
    word wStart;
    word wEnd;
    if(end<start){
        return 0;
    }
    wStart = start >> WORD_SHIFT;
    wEnd = end>> WORD_SHIFT;
    start = start & WORD_MOD;
    end = end & WORD_MOD;
    if(wStart == wEnd){
        extracted = (B->v[wStart] & maskRangeAccess[start][end]) >> (WORD_SIZE-end-1);
    }
    else{
        h = B->v[wStart] & maskRangeAccess[start][WORD_SIZE-1];
        h <<= end+1;
        l = (B->v[wEnd] & maskRangeAccess[0][end]);
        l >>=  (WORD_SIZE-end-1);
        extracted = h | l;
    }
    return(extracted);
}


void resetBitVector(bitVector* B){
    bzero(B->v,sizeof(word)*B->words);
}




/*****  UTILITIES FUNCTIONS *****/





word concatenateWords(word a, word b,word nBitsB){
    a <<= nBitsB;
    return(a|b);
}

word selectBitsFromWord(word a,int i,int j){
    if(j<i) return 0;
    return(a & maskRangeAccess[i][j]);
}

word retrieveNumBits(word n){
    return (n==0 ? 1 : floor(log2((double)n))+1 );
}

void saveBitVector(bitVector* B, FILE* ptr){
    word zero = 0;
    word one  = 1;
    if(B==NULL){
        fwrite(&zero,sizeof(word),1,ptr);
    }
    else{
        fwrite(&one,sizeof(word),1,ptr);
        fwrite(&(B->size),sizeof(word),1,ptr);
        fwrite(&(B->words),sizeof(word),1,ptr);
        fwrite(&(B->rankWords),sizeof(word),1,ptr);
        if(B->words>0)
            fwrite(B->v,sizeof(word),B->words,ptr);
        if(B->rankWords>0)
            fwrite(B->rankLookUp,sizeof(word),B->rankWords,ptr);
    }
}

bitVector* loadBitVector(FILE* ptr){
    word status;
    bitVector* B;
    fread(&status,sizeof(word),1,ptr);
    if(status==0) return NULL;
    B = callocx(sizeof(bitVector),1);
    fread(&(B->size),sizeof(word),1,ptr);
    fread(&(B->words),sizeof(word),1,ptr);
    fread(&(B->rankWords),sizeof(word),1,ptr);
    B->v=NULL;
    B->rankLookUp=NULL;
    if(B->words>0){
        B->v = callocx(sizeof(word),B->words);
        fread(B->v,sizeof(word),B->words,ptr);
    }
    if(B->rankWords>0){
        B->rankLookUp = callocx(sizeof(word),B->rankWords);
        fread(B->rankLookUp,sizeof(word),B->rankWords,ptr);
    }
    return(B);
}
