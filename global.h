#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include <stdio.h>

typedef struct indexConfiguration{
    int SA_SAMPLE_RATE;
    int SELECT_FACTOR;
    int ALPHABET_SIZE;
}indexConfiguration;

extern indexConfiguration indexConfig;

void readConfigFile(char* fileName){
}


indexConfig* loadIndexConfiguration(file* ptr);
void writeIndexConfiguration(file* ptr);
void readIndexConfiguration(FILE* ptr);
void freeIndexConfiguration(void);
#endif // GLOBAL_H_INCLUDED
