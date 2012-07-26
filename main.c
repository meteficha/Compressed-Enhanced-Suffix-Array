#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csa.h"
#include "search.h"
#include "alloc.h"
#include "bit.h"
#include "text.h"
/**ARGV[1]
    FILE CONTAINING THE TEXT
**/

int main(int argc,char** argv){
    word i;
    text* T = callocx(sizeof(text),1);
    text* P = callocx(sizeof(text),1);
    cesa* CSA;
    cesa* CSA2;
    if(argc<4 || argc >5){
        fprintf(stderr,"%s","Error\n");
        fprintf(stderr,"%s%s%s","Usage = ",argv[0]," -create <inputText> <indexFile>\n");
        fprintf(stderr,"%s%s%s","Usage = ",argv[0]," -search <indexFile> <inputText> <patternFile>\n");
        exit(EXIT_FAILURE);
    }
    /**preprocess step**/
    initAllBitTables();

    if(strcmp(argv[1],"-create")==0){
        T->file = fopen(argv[2],"r");
        if(T->file==NULL){
            perror("Error: ");
            exit(EXIT_FAILURE);
        }
        fseek(T->file,0,SEEK_END);
        T->length = ftell(T->file);
        rewind(T->file);
        T->textStart = 0;
        T->textEnd = T->length-1;
        CSA = computeCSA(T);
        computeLcp(CSA);
        saveIndex(CSA,argv[3]);
    }
    else if(strcmp(argv[1],"-search")==0){
        P->file = fopen(argv[4],"r");
        T->file = fopen(argv[3],"r");
        if(T->file==NULL){
            perror("Error: ");
            exit(EXIT_FAILURE);
        }
        if(P->file == NULL){
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
        CSA = loadIndex(argv[2]);
        CSA->P = P;
        CSA->T = T;
        searchPattern(CSA);
    }
    else{
        fprintf(stderr,"%s%s%s","Usage = ",argv[0]," -create <inputText> <indexFile>\n");
        fprintf(stderr,"%s%s%s","Usage = ",argv[0]," -search <indexFile> <inputText> <patternFile>\n");
        exit(EXIT_FAILURE);
    }
    freeText(T);
    freeText(P);
    freeCSA(CSA);
    return(0);
}
