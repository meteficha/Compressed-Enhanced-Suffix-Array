#include <math.h>
#include <string.h>
#include <assert.h>
#include "csa.h"
#include "utils.h"
#include "suffixSort.h"
#include "alloc.h"

static word BLOCKSIZE; /**Size of the Block**/
static word LCP_BLOCKSIZE = 1<<8;
static word* WM; /**work Memory**/
static word* M; /** Rank of Long suffixes among themselves**/
static word* MI; /** Inverse of M**/
static word* L; /** Rank of Long suffixes among all suffixes**/
static word* LSorted; /** Sorted list of the Rank of Long suffixes among all suffixes**/
static bitVector* V; /**bit vector, V[i]=1 if the ith suffix in lexicographical order is a long suffix, otherwise, V[i]=0**/
cesa* tempCSA; /**temporary CSA**/
byte* previousTextBlock;
byte* currentTextBlock;
byte* textBlock;

byte* lcpBlock1;
byte* lcpBlock2;

cesa* computeCSA(text* T){

    word start,end; /**marks the beggining and the end of the current block**/
    word currentBlockSize; /**Keeps the current block size**/
    word i,it;
    cesa* CSA; /**The compressed Suffix Array**/

    CSA = createCSA(T);
    max(1024,floor(T->length/(4*log2(T->length))),BLOCKSIZE);
    it = ceil((double)T->length/BLOCKSIZE); /**number of iterations**/
    WM = callocx((BLOCKSIZE+1)*4,sizeof(word)); /**Working memory**/
    V = createBitVector(T->length); /**V[i] = 1 if the ith suffix is a long suffix, short suffix otherwise**/
    currentTextBlock = callocx(sizeof(byte),BLOCKSIZE);
    previousTextBlock = callocx(sizeof(byte),BLOCKSIZE);


    /**BASE CASE**/
    end = T->length-1;
    start = BLOCKSIZE > end ? 0 : end - BLOCKSIZE + 1;
    currentBlockSize = end-start+1;
    readBufferFromText(CSA->T,currentTextBlock,start,BLOCKSIZE);
    baseCase(CSA,currentBlockSize);
    textBlock = previousTextBlock;
    previousTextBlock = currentTextBlock;
    currentTextBlock = textBlock;
    for(i=1;i<it;i++){
        /**seting boundaries**/
        end=start-1;
        start = BLOCKSIZE > end ? 0 : end - BLOCKSIZE + 1;
        currentBlockSize = end - start +1;
        readBufferFromText(CSA->T,currentTextBlock,start,BLOCKSIZE);

        /**Compute the rank of long suffixes among themselves**/
        computeRankAmongLongSuffixes(CSA,currentBlockSize);

        /**Compute rank of long suffixes among the short suffixes**/
        computeRankAmongShortSuffixes(CSA,currentBlockSize);

        /**COMPUTE PSI**/
        computeNewPsi(CSA,currentBlockSize,start);

        textBlock = previousTextBlock;
        previousTextBlock = currentTextBlock;
        currentTextBlock = textBlock;
    }
    /**Deallocate space**/
    free(currentTextBlock);
    free(previousTextBlock);
    freeBitVector(V);
    free(WM);
    freeCSA(tempCSA);
    sampleSA(CSA);
    return(CSA);
}

/**Returns the maximum r, such that Psi(r)<= Rank A_{k+1}**/

word rankBsearch(cesa* CSA,word rankNextSuffix,word ch){
    word rank=0;
    word m;
    word tempRank;
    word l,r;
    l = CSA->c[ch];
    r = CSA->c[ch+1]-1;
    rank = CSA->c[ch];
    while(l<r){
        m = (l+r)/2;
        tempRank = accessPsi(CSA,m);
        if(tempRank<rankNextSuffix){
            rank=m+1;
            l=m+1;
        }
        else if (tempRank==rankNextSuffix){
            rank = m;
            l=m+1;
        }
        else{
            r=m-1;
        }
    }
    if(l==r){
        tempRank = accessPsi(CSA,l);
         if(tempRank<rankNextSuffix){
            rank = l+1;
        }
        else if(tempRank==rankNextSuffix){
            rank = l;
        }
    }
    return(rank);
}

void computeRankAmongShortSuffixes(cesa* CSA,word currentBlockSize){
    word ch;
    word i;
    L=&WM[2*BLOCKSIZE];
    LSorted = &WM[3*BLOCKSIZE];
    for(i=0;i<=ALPHABET_SIZE;i++) tempCSA->c[i]=0;
    i = currentBlockSize;
    L[i]=accessPsi(CSA,0);
    while(i!=0){
        i--;
        /**Compute a binary Search**/
        ch=currentTextBlock[i];
        (tempCSA->c[ch+1])++;
        L[i] = rankBsearch(CSA,L[i+1],ch);
        LSorted[i]=L[i]; /**copy elements**/
    }
    qsort(LSorted,currentBlockSize,sizeof(word),compareWords);
}

void computeRankAmongLongSuffixes(cesa* CSA, word currentBlockSize){

    word i;
    word* SA; /**suffix array of the considered block**/
    word* SAI; /**suffix array of the considered block**/
    suffixStructure* tuples;

    SAI = WM;
    SA = &WM[BLOCKSIZE*2+1];
    for(i=0;i<currentBlockSize;i++){
        SAI[i] = (word) currentTextBlock[i];
    }

    for(i=currentBlockSize;i<currentBlockSize*2;i++){
        SAI[i] = (word) previousTextBlock[i-currentBlockSize];
    }
    suffixSort((integer*)SAI,(integer*)SA,currentBlockSize*2,ALPHABET_SIZE,1,currentBlockSize);

    /**FILL P and Q **/
    tuples = (suffixStructure*) &WM[BLOCKSIZE];
    tuples[0].Q = accessPsi(CSA,0);
    tuples[0].P = SAI[0];
    tuples[0].index = 0;
    for(i=1;i<currentBlockSize;i++){
        tuples[i].index = i;
        tuples[i].P = SAI[i];
        tuples[i].Q = accessPsi(CSA,tuples[i-1].Q);
    }
    /**Sort structure**/
    qsort(tuples,currentBlockSize,sizeof(suffixStructure),compareSuffixes);
    /**set M and MI**/
    M=WM;
    MI = &WM[BLOCKSIZE];
    for(i=0;i<currentBlockSize;i++) M[tuples[i].index] = i;
    for(i=0;i<currentBlockSize;i++) MI[M[i]] = i;
}

/**TODO: MODIFY TO OVERWRITE THE OLD PSI WHILE CREATING THE NEW ONE**/


void computeNewPsi(cesa* CSA,word currentBlockSize,word start){
    word i,it;
    word rank;
    word suffix;
    word psiValue;
    psiStructure* temp;
    word lastSetBit;
    resetBitVector(V);

    /**configure new psi**/
    for(i=1;i<=ALPHABET_SIZE;i++) tempCSA->c[i] += tempCSA->c[i-1];
    for(i=0;i<=ALPHABET_SIZE;i++) tempCSA->c[i] += CSA->c[i];

    setBit(V,LSorted[0]);
    lastSetBit = LSorted[0];
    for(i=1;i<currentBlockSize;i++){
        setBit(V,lastSetBit+LSorted[i]-LSorted[i-1]+1);
        lastSetBit += LSorted[i]-LSorted[i-1]+1;
    }
    preprocessRank(V);

    it = CSA->T->length - start;
    configurePsi(tempCSA);
    setPsiValue(tempCSA,0,M[0]+L[0]);
    for(i=1;i<it;i++){
        if(readBitFromBitVector(V,i)==0){ /**Is a short suffix**/
            rank = rank0(V,i)-1;
            rank = accessPsi(CSA,rank);
            psiValue = select0(V,rank+1);
        }
        else{ /**Is a long suffix**/
            rank = rank1(V,i)-1;
            suffix = MI[rank];
            if(suffix<currentBlockSize-1){
                psiValue = M[suffix+1]+L[suffix+1];
            }
            else{
                rank = accessPsi(CSA,0)+1;
                psiValue = select0(V,rank);
            }
        }
        setPsiValue(tempCSA,i,psiValue);
    }
    temp = CSA->psi;
    CSA->psi = tempCSA->psi;
    tempCSA->psi = temp;
    for(i=1;i<=ALPHABET_SIZE;i++) CSA->c[i]= tempCSA->c[i];
    preprocessQuotientRank(CSA);
}


cesa* createCSA(text* T){
    word i;
    cesa* CSA = callocx(1,sizeof(cesa));
    tempCSA = callocx(1,sizeof(cesa));
    /**allocate space for the psi vectors**/
    CSA->psi = callocx(1,sizeof(psiStructure));
    CSA->sa = NULL;
    CSA->lcp = NULL;
    for(i=0;i<ALPHABET_SIZE;i++){
        CSA->psi->psiRemainder[i] = createBitVector(0);
        CSA->psi->psiQuotient[i] = createBitVector(0);
    }
    tempCSA->psi = callocx(1,sizeof(psiStructure));
    for(i=0;i<ALPHABET_SIZE;i++){
        tempCSA->psi->psiRemainder[i] = createBitVector(0);
        tempCSA->psi->psiQuotient[i] = createBitVector(0);
    }
    CSA->n = T->length;
    CSA->T = T;
    tempCSA->n = T->length;
    tempCSA->T = T;
    tempCSA->sa = NULL;
    tempCSA->lcp = NULL;
    return(CSA);
}

void baseCase(cesa* CSA,word currentBlockSize){
    word i;
    word *SA,*SAI,*psiValues;
    word symbolCount,temp;
    SAI = WM;
    SA = &WM[BLOCKSIZE+2];

    for(i=0;i<=ALPHABET_SIZE;i++) CSA->c[i]=0;
    for(i=0;i<currentBlockSize;i++){
        SAI[i] = (word) currentTextBlock[i];
        CSA->c[SAI[i]] = CSA->c[SAI[i]]+1;
    }
    SAI[i]=0;
    symbolCount=CSA->c[0];
    CSA->c[0]=0;
    for(i=1;i<=ALPHABET_SIZE;i++){
        temp = CSA->c[i];
        CSA->c[i] = CSA->c[i-1]+symbolCount;
        symbolCount = temp;
    }
    suffixSort((integer*)SAI,(integer*)SA,currentBlockSize,ALPHABET_SIZE,1,currentBlockSize);

    for(i=0;i<currentBlockSize;i++){
        SA[SAI[i]] = i;
    }
    configurePsi(CSA);
    psiValues = &WM[3*BLOCKSIZE];
    psiValues[0] = SAI[0];

    for(i=1;i<currentBlockSize;i++){
        psiValues[i] = SAI[SA[i] + 1];
    }
    /**construct the psi bit vector**/
    for(i=0;i<currentBlockSize;i++){
        setPsiValue(CSA,i,psiValues[i]);
    }
    preprocessQuotientRank(CSA);
}

word accessPsi(cesa* CSA,word i){
    /**use binary search to find the alphabet symbol of position i**/
    bitVector* remainderV,*quotientV;
    word k,l,r,m,ch;
    word nBits;
    word quotient,remainder;

    l = 0;
    r = ALPHABET_SIZE-1;
    ch  = l;
    while(l<=r){
        m = (l+r)/2;
        if(CSA->c[m]<=i){
            l = m+1;
            ch = m;
        }
        else{
            r = m-1;
        }
    }
    k = i-CSA->c[ch];
    remainderV = CSA->psi->psiRemainder[ch];
    quotientV  = CSA->psi->psiQuotient[ch];
    nBits = CSA->psi->psiRemainderBits[ch];
    quotient = select1(quotientV,k+1)-(k);
    if(nBits>0){
        remainder = extractBitsFromBitVector(remainderV,nBits*k,nBits*(k+1)-1);
    }
    else{
        remainder = 0;
    }
    return(concatenateWords(quotient,remainder,CSA->psi->psiRemainderBits[ch]));
}


void setPsiValue(cesa* CSA,word i ,word value){
    word k,l,r,m;
    word start;
    word ch;
    word diff;
    word quotient;
    word remainder;

    l = 0;
    r = ALPHABET_SIZE-1;
    ch  = l;
    while(l<=r){
        m = (l+r)/2;
        if(CSA->c[m]<=i){
            l = m+1;
            ch = m;
        }
        else{
            r = m-1;
        }
    }
    start = WORD_SIZE-CSA->psi->currentWordSize;
    quotient = selectBitsFromWord(value,start,start+CSA->psi->psiQuotientBits[ch]-1) >> (CSA->psi->currentWordSize-CSA->psi->psiQuotientBits[ch]);
    k = i-CSA->c[ch];
    remainder = selectBitsFromWord(value,start+CSA->psi->psiQuotientBits[ch],WORD_SIZE-1);
    diff = quotient - CSA->psi->lastCodedPsiQuotient[ch];
    CSA->psi->lastCodedPsiQuotient[ch]=quotient;
    setBit(CSA->psi->psiQuotient[ch],CSA->psi->lastQuotientBit[ch]+diff+1);
    CSA->psi->lastQuotientBit[ch]+=diff+1;
    if(CSA->psi->psiRemainderBits[ch]>0)
        setBitsInBitVector(CSA->psi->psiRemainder[ch],remainder,CSA->psi->psiRemainderBits[ch],
                           k*CSA->psi->psiRemainderBits[ch]);
}


/**Configure the psi structure**/
void configurePsi(cesa* CSA){
    psiStructure* psi;
    word i;
    word symbols;
    word logC;
    word remainderBits;
    psi = CSA->psi;

    /**TODO: LITTLE BUG WHEN SYMBOLS=1**/
    psi->currentWordSize = retrieveNumBits(CSA->c[ALPHABET_SIZE]);
    for(i=1;i<=ALPHABET_SIZE;i++){
        symbols = CSA->c[i]-CSA->c[i-1];
        if(symbols>0){
            max(floor(log2(symbols)),1,logC);
            remainderBits = psi->currentWordSize-logC;
            psi->psiQuotientBits[i-1] = logC;
            psi->psiRemainderBits[i-1] = remainderBits;
            psi->lastQuotientBit[i-1] = MAX_WORD;
            psi->lastCodedPsiQuotient[i-1] = 0;
            /**resize bit vector**/
            if(psi->psiQuotient[i-1]->size < 2*symbols)
                resizeBitVector(psi->psiQuotient[i-1],2*symbols);
            if(psi->psiRemainder[i-1]->size < (psi->psiRemainderBits[i-1])*symbols)
                resizeBitVector(psi->psiRemainder[i-1],(psi->psiRemainderBits[i-1])*symbols);
            resetBitVector(psi->psiQuotient[i-1]);
            resetBitVector(psi->psiRemainder[i-1]);
        }
    }
}

void preprocessQuotientRank(cesa* CSA){
    word i;
    for(i=0;i<ALPHABET_SIZE;i++){
        preprocessRank(CSA->psi->psiQuotient[i]);
    }
}

void sampleSA(cesa* CSA){
    word i;
    word suffixValue,rank;
    CSA->sa = callocx(sizeof(saStructure),1);
    CSA->sa->sampleRate = SA_SAMPLE_RATE;
    CSA->sa->words = 1+ floor(CSA->n/CSA->sa->sampleRate);
    CSA->sa->array = callocx(sizeof(word),1+ floor(CSA->n/CSA->sa->sampleRate));
    CSA->sa->array[0] = CSA->n-1;
    suffixValue=CSA->n-1;
    rank = 0;
    for(i=1;i<CSA->n;i++){
        rank = accessPsi(CSA,rank);
        suffixValue = (suffixValue+1) % CSA->n;
        if(rank%CSA->sa->sampleRate==0){
            CSA->sa->array[rank/CSA->sa->sampleRate]= suffixValue;
        }
    }
}

word accessSA(cesa* CSA,word i){
    word t;
    word rank;
    if(i%CSA->sa->sampleRate==0){
        return(CSA->sa->array[i/CSA->sa->sampleRate]);
    }
    else{
        t=0;
        rank = i;
        while(rank%CSA->sa->sampleRate!=0){
            rank = accessPsi(CSA,rank);
            t++;
        }
        if(t>CSA->sa->array[rank/CSA->sa->sampleRate]){
            return(CSA->n - CSA->sa->array[rank/CSA->sa->sampleRate] + t );
        }
        return(CSA->sa->array[rank/CSA->sa->sampleRate] - t);
    }
}

void freeCSA(cesa* CSA){
    word i;
    for(i=0;i<ALPHABET_SIZE;i++){
        if(CSA->psi->psiQuotient[i]!=NULL)
            freeBitVector(CSA->psi->psiQuotient[i]);
        if(CSA->psi->psiRemainder[i]!=NULL)
            freeBitVector(CSA->psi->psiRemainder[i]);
    }
    free(CSA->psi);
    if(CSA->sa!=NULL){
        free(CSA->sa->array);
        free(CSA->sa);
    }
    if(CSA->lcp!=NULL){
        freeBitVector(CSA->lcp->lcpQuotient);
        free(CSA->lcp);
    }
    free(CSA);
}




void createLcp(cesa* CSA){
    CSA->lcp=callocx(sizeof(lcpStructure),1);
    CSA->lcp->lcpQuotient = createBitVector(2*CSA->n);
    CSA->lcp->lastCodedLcpQuotient = 0;
    CSA->lcp->lastQuotientBit = MAX_WORD;
    resetBitVector(CSA->lcp->lcpQuotient);
    lcpBlock1 = callocx(sizeof(byte),LCP_BLOCKSIZE);
    lcpBlock2 = callocx(sizeof(byte),LCP_BLOCKSIZE);
}

void setLcpValue(cesa* CSA, word saValue,word lcpValue){
    word diff;
    word value = saValue+lcpValue;
    diff = value - CSA->lcp->lastCodedLcpQuotient;
    CSA->lcp->lastCodedLcpQuotient = value;
    setBit(CSA->lcp->lcpQuotient,CSA->lcp->lastQuotientBit+diff+1);
    CSA->lcp->lastQuotientBit+=diff+1;
}


word accessLcp(cesa* CSA, word i){
    word saEntry = accessSA(CSA,i);
    word lcpSA = select1(CSA->lcp->lcpQuotient,saEntry+1);
    return(lcpSA - 2*saEntry);
}


/**Use kasai et al algorithm
    lcp[i] = lcp(S_{A[i]},S_{A[i+1]})
**/

void computeLcp(cesa* CSA){
    word i,h,sufInv;
    word k;
    createLcp(CSA);
    sufInv = 0;
    h=0;
    for(i=0;i<CSA->n;i++){
        sufInv = accessPsi(CSA,sufInv);
        if(sufInv !=  CSA->n-1){
            k = accessSA(CSA,sufInv+1);
            h = lcpCompare(CSA,i,k,h);
            setLcpValue(CSA,accessSA(CSA,sufInv),h);
            if(h>0) h--;
        }
        else{
            setLcpValue(CSA,accessSA(CSA,sufInv),0);
        }
    }
    preprocessRank(CSA->lcp->lcpQuotient);
    free(lcpBlock1);
    free(lcpBlock2);

}

word lcpCompare(cesa* CSA,word i,word k,word h){
    word currentBlock = h/LCP_BLOCKSIZE;
    word start = h % LCP_BLOCKSIZE;
    readBufferFromText(CSA->T,lcpBlock1,i+currentBlock*LCP_BLOCKSIZE,LCP_BLOCKSIZE);
    readBufferFromText(CSA->T,lcpBlock2,k+currentBlock*LCP_BLOCKSIZE,LCP_BLOCKSIZE);
    while(lcpBlock1[start]==lcpBlock2[start]){
        h++;
        start++;
        if(start==LCP_BLOCKSIZE){
            currentBlock++;
            readBufferFromText(CSA->T,lcpBlock1,i+currentBlock*LCP_BLOCKSIZE,LCP_BLOCKSIZE);
            readBufferFromText(CSA->T,lcpBlock2,k+currentBlock*LCP_BLOCKSIZE,LCP_BLOCKSIZE);
            start = 0;
        }
    }
    return(h);

}


void saveIndex(cesa* CSA,char* filename){
    word zero=0;
    word one=1;
    word i;
    CSA->Index = fopen(filename,"wb");
    if(CSA->Index==NULL){
        perror("Error: ");
        exit(EXIT_FAILURE);
    }
    /**IF sa==NULL writes 0, otherwise writes 1**/
    if(CSA->sa==NULL){
        fwrite(&zero,sizeof(word),1,CSA->Index);
    }
    else{
        fwrite(&one,sizeof(word),1,CSA->Index); /**write 0**/
        fwrite(&(CSA->sa->words),sizeof(word),1,CSA->Index); /**write the number of the compressed sa elements**/
        fwrite(CSA->sa->array,sizeof(word),CSA->sa->words,CSA->Index); /**write the sa elements**/
        fwrite(&(CSA->sa->sampleRate),sizeof(word),1,CSA->Index); /**write the sample rate**/
    }

    /**IF lcp==NULL writes zero, othewrise writes one**/
    if(CSA->lcp==NULL)
        fwrite(&zero,sizeof(word),1,CSA->Index);
    else{
        fwrite(&one,sizeof(word),1,CSA->Index); /**write 1**/
        saveBitVector(CSA->lcp->lcpQuotient,CSA->Index); /**Save bitVector**/
        fwrite(&(CSA->lcp->lastCodedLcpQuotient),sizeof(word),1,CSA->Index); /**Save lastCodedLcpQuotient**/
        fwrite(&(CSA->lcp->lastQuotientBit),sizeof(word),1,CSA->Index); /**Save lastQuotientBit**/
    }
    for(i=0;i<ALPHABET_SIZE;i++){
        saveBitVector(CSA->psi->psiQuotient[i],CSA->Index);
    }
    for(i=0;i<ALPHABET_SIZE;i++){
        saveBitVector(CSA->psi->psiRemainder[i],CSA->Index);
    }
    fwrite(CSA->psi->psiRemainderBits,sizeof(word),ALPHABET_SIZE,CSA->Index);
    fwrite(CSA->psi->psiQuotientBits,sizeof(word),ALPHABET_SIZE,CSA->Index);
    fwrite(CSA->psi->lastCodedPsiQuotient,sizeof(word),ALPHABET_SIZE,CSA->Index);
    fwrite(CSA->psi->lastQuotientBit,sizeof(word),ALPHABET_SIZE,CSA->Index);
    fwrite(&(CSA->psi->currentWordSize),sizeof(word),1,CSA->Index);


    fwrite(CSA->c,sizeof(word),ALPHABET_SIZE+1,CSA->Index);
    fwrite(&(CSA->n),sizeof(word),1,CSA->Index);
    fclose(CSA->Index);

}

cesa* loadIndex(char* filename){
    cesa* CSA;
    FILE* ptr;
    word status,i;
    ptr = fopen(filename,"rb");
    if(ptr==NULL){
        perror("Could not load the index: ");
        exit(EXIT_FAILURE);
    }
    CSA = callocx(sizeof(cesa),1);
    CSA->Index = ptr;

    /**read sa information**/
    fread(&status,sizeof(word),1,CSA->Index);
    if(status==0){
        CSA->sa = NULL;
    }
    else{
        CSA->sa=callocx(sizeof(saStructure),1);
        fread(&(CSA->sa->words),sizeof(word),1,CSA->Index);
        CSA->sa->array=callocx(sizeof(word),CSA->sa->words);
        fread(CSA->sa->array,sizeof(word),CSA->sa->words,CSA->Index);
        fread(&(CSA->sa->sampleRate),sizeof(word),1,CSA->Index);
    }

    fread(&status,sizeof(word),1,CSA->Index);
    if(status==0){
        CSA->lcp = NULL;
    }
    else{
        CSA->lcp = callocx(sizeof(lcpStructure),1);
        CSA->lcp->lcpQuotient = loadBitVector(CSA->Index);
        fread(&(CSA->lcp->lastCodedLcpQuotient),sizeof(word),1,CSA->Index);
        fread(&(CSA->lcp->lastQuotientBit),sizeof(word),1,CSA->Index);
    }

    CSA->psi = callocx(sizeof(psiStructure),1);
    for(i=0;i<ALPHABET_SIZE;i++){
        CSA->psi->psiQuotient[i] = loadBitVector(CSA->Index);
    }
    for(i=0;i<ALPHABET_SIZE;i++){
        CSA->psi->psiRemainder[i] = loadBitVector(CSA->Index);
    }
    fread(CSA->psi->psiRemainderBits,sizeof(word),ALPHABET_SIZE,CSA->Index);
    fread(CSA->psi->psiQuotientBits,sizeof(word),ALPHABET_SIZE,CSA->Index);
    fread(CSA->psi->lastCodedPsiQuotient,sizeof(word),ALPHABET_SIZE,CSA->Index);
    fread(CSA->psi->lastQuotientBit,sizeof(word),ALPHABET_SIZE,CSA->Index);
    fread(&(CSA->psi->currentWordSize),sizeof(word),1,CSA->Index);


    fread((CSA->c),sizeof(word),ALPHABET_SIZE+1,CSA->Index);
    fread(&(CSA->n),sizeof(word),1,CSA->Index);

    fclose(CSA->Index);
    return(CSA);


}


