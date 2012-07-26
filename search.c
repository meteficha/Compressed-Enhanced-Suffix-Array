#include "search.h"
#include "alloc.h"

static char* pattern;
static char* textBlock;

void searchPattern(cesa* CSA){
    word l,r,m,i;
    word found=0;
    pattern = callocx(sizeof(byte),CSA->P->length);
    textBlock = callocx(sizeof(byte),CSA->P->length);
    fseek(CSA->P->file,0,SEEK_SET);
    fread(pattern,sizeof(byte),CSA->P->length,CSA->P->file);
    l = 0;
    r = CSA->n - 1;
    if(comparePattern(CSA,0)<=0){
        l=0;
        if(comparePattern(CSA,0)==0){
            found = 1;
        }
    }
    else if(comparePattern(CSA,CSA->n-1)>0){
        l=CSA->n;
    }
    else{
        while(l<r){
            m = (l+r)/2;
            if(comparePattern(CSA,m)<=0){
                r = m;
            }
            else{
                l = m+1;
            }
        }
        if(l==r){
            if(comparePattern(CSA,l)==0){
                found = 1;
            }
            else{
                found = 0;
            }
        }
    }

    if(found==1){
        printf("%"PRINT_WORD"\n",accessSA(CSA,l));
        i=l;
        while(i<CSA->n-1 && accessLcp(CSA,i)>=CSA->P->length){
            printf("%"PRINT_WORD"\n",accessSA(CSA,i+1));
            i++;
        }
    }
    else{
        printf("No ocurrences found");
    }
    free(pattern);
    free(textBlock);
}

int comparePattern(cesa* CSA,word i){
    word k;
    word compare = 0;
    word suffix = accessSA(CSA,i);
    fseek(CSA->T->file,suffix,SEEK_SET);
    fread(textBlock,sizeof(byte),CSA->P->length,CSA->T->file);
    k=0;
    while(k<CSA->P->length){
        if(pattern[k]<textBlock[k]){
            compare = -1;
            break;
        }
        else if(pattern[k]>textBlock[k]){
            compare=1;
            break;
        }
        k++;
    }
    return(compare);
}
