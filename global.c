#include <stdlib.h>
#include "global.h"

indexConfiguration* indexConfig=NULL;

static FILE* configFile;

indexConfiguration loadIndexConfiguration(FILE* PTR){
    indexConfiguration* ic = callocx(sizeof(indexConfiguration),1);
    fread(&(ic->SA_SAMPLE_RATE),sizeof(int),1,ptr);
    fread(&(ic->SELECT_FACTOR),sizeof(int),1,ptr);
    fread(&(ic->ALPHABET_SIZE),sizeof(int),1,ptr);
    return(ic);
}

void readIndexConfiguration(char* fileName){
    char str[250];
    int v;
    configFile=fopen(filename,"r");
    if(configFile==NULL){
        perror("Error");
        exit(EXIT_FAILURE);
    }
    indexConfig = callocx(sizeof(indexConfiguration),1);
    fgets(str,250,ptr);
    /**read first field**/
    fscanf(ptr,"%s %d %s",str,&v,str);
    indexConfig->SA_SAMPLE_RATE = v;
    /**read second field**/
    fscanf(ptr,"%s %d %s",str,&v,str);
    indexConfig->SELECT_FACTOR = v;
    /**read third field**/
    fscanf(ptr,"%s %d %s",str,&v,str);
    indexConfig->ALPHABET_SIZE = v;
}

void writeIndexConfiguration(FILE* ptr){
    fwrite(&(indexConfig->SA_SAMPLE_RATE),sizeof(int),1,ptr);
    fwrite(&(indexConfig->SELECT_FACTOR),sizeof(int),1,ptr);
    fwrite(&(indexConfig->ALPHABET_SIZE),sizeof(int),1,ptr);
}

void freeIndexConfiguration(void){
    if(indexConfig!=NULL)
        free(indexConfig);
}


