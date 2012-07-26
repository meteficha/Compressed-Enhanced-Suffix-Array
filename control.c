#include <stdio.h>
#include <stdlib.h>
#include <divsufsort.h>
#include <stdlib.h>
#include <string.h>
#include "alloc.h"
#include "text.h"
#include "types.h"

static char* pattern;
static char* textBlock;



int comparePattern(text* T,text* P,saidx_t* SA,word i){
    word k;
    word compare = 0;
    word suffix = SA[i];
    fseek(T->file,suffix,SEEK_SET);
    fread(textBlock,sizeof(byte),P->length,T->file);
    k=0;
    while(k<P->length){
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

void searchPattern(text* T, text* P,saidx_t* SA,saidx_t* LCP, word n){
    word l,r,m,i;
    word found=0;
    pattern = callocx(sizeof(byte),P->length);
    textBlock = callocx(sizeof(byte),P->length);
    fseek(P->file,0,SEEK_SET);
    fread(pattern,sizeof(byte),P->length,P->file);
    l = 0;
    r = n - 1;
    if(comparePattern(T,P,SA,0)<=0){
        l=0;
        if(comparePattern(T,P,SA,0)==0){
            found = 1;
        }
    }
    else if(comparePattern(T,P,SA,n-1)>0){
        l=n;
    }
    else{
        while(l<r){
            m = (l+r)/2;
            if(comparePattern(T,P,SA,m)<=0){
                r = m;
            }
            else{
                l = m+1;
            }
        }
        if(l==r){
            if(comparePattern(T,P,SA,l)==0){
                found = 1;
            }
            else{
                found = 0;
            }
        }
    }

    if(found==1){
        printf("%"PRId32"\n",SA[l]);
        i=l;
        while(i<n-1 && LCP[i]>=P->length){
            printf("%"PRId32"\n",SA[i+1]);
            i++;
        }
    }
    else{
        printf("No ocurrences found");
    }
    free(pattern);
    free(textBlock);
}




int main(int argc,char** argv){
	sauchar_t* str;
	saidx_t* SA;
	saidx_t* SAI;
	word n;
	saidx_t* LCP;
	int h,j,k;
    int i;
    text* T = callocx(1,sizeof(text));
    text* P = callocx(1,sizeof(text));
    text* Index = callocx(1,sizeof(text));
    if(argc<4 || argc >5){
        fprintf(stderr,"%s","Error\n");
        fprintf(stderr,"%s%s%s","Usage = ",argv[0]," -create <inputText> <indexFile>\n");
        fprintf(stderr,"%s%s%s","Usage = ",argv[0]," -search <indexFile> <inputText> <patternFile>\n");
        exit(EXIT_FAILURE);
    }
    if(strcmp(argv[1],"-create")==0){
        T->file = fopen(argv[2],"r");
        Index->file = fopen(argv[3],"wb");

        if(T->file==NULL){
            perror("Error: ");
            exit(EXIT_FAILURE);
        }
        if(Index->file==NULL){
            perror("Error: ");
            exit(EXIT_FAILURE);
        }

        fseek(T->file,0,SEEK_END);
        T->length = ftell(T->file);
        rewind(T->file);

        str = callocx(sizeof(sauchar_t),T->length);
        SA = callocx(sizeof(saidx_t),T->length);
        SAI = callocx(sizeof(saidx_t),T->length);
        LCP = callocx(sizeof(saidx_t),T->length);
        fread(str,sizeof(sauchar_t),T->length,T->file);
        if(divsufsort(str,SA,T->length)){
            perror("suffix sort error");
            exit(EXIT_FAILURE);
        }

        for(i=0;i<T->length;i++){
            SAI[SA[i]] = i;
        }

        for(i=0;i<T->length;i++){
            h=0;
            if(SAI[i] !=  T->length -1){
                k = SA[SAI[i]+1];
                j=0;
                while(str[i+h]==str[k+h]) h++;
                LCP[SAI[i]] = h;
                if(h) h--;
            }
            else{
                LCP[SAI[i]] = 0;
            }
        }
        if(Index->file==NULL){
            perror("Error");
            exit(EXIT_FAILURE);
        }
        fwrite(&(T->length),sizeof(word),1,Index->file);
        fwrite(SA,sizeof(word),T->length,Index->file);
        fwrite(LCP,sizeof(word),T->length,Index->file);
        fclose(Index->file);
        free(SA);
        free(SAI);
        free (LCP);
        free(str);

    }
    else if(strcmp(argv[1],"-search")==0){
        P->file = fopen(argv[4],"r");
        T->file = fopen(argv[3],"r");
        Index->file = fopen(argv[2],"rb");
        if(T->file==NULL){
            perror("Error: ");
            exit(EXIT_FAILURE);
        }
        if(P->file == NULL){
            perror("Error: ");
            exit(EXIT_FAILURE);
        }
        if(Index->file==NULL){
            perror("Error: ");
            exit(EXIT_FAILURE);
        }
        fseek(T->file,0,SEEK_END);
        T->length = ftell(T->file);
        rewind(T->file);
        T->textStart = 0;
        T->textEnd = T->length-1;
        fseek(P->file,0,SEEK_END);
        P->length = ftell(P->file);
        rewind(P->file);
        fread(&n,sizeof(word),1,Index->file);
        SA = callocx(sizeof(saidx_t),n);
        LCP = callocx(sizeof(saidx_t),n);
        fread(SA,sizeof(saidx_t),n,Index->file);
        fread(LCP,sizeof(saidx_t),n,Index->file);
        searchPattern(T,P,SA,LCP,n);
        free(SA);
        free(LCP);
    }
    else{
        fprintf(stderr,"%s%s%s","Usage = ",argv[0]," -create <inputText> <indexFile>\n");
        fprintf(stderr,"%s%s%s","Usage = ",argv[0]," -search <inputText> <indexFile> <patternFile>\n");
        exit(EXIT_FAILURE);
    }
    freeText(T);
    freeText(P);
    freeText(Index);
    return(0);
}
