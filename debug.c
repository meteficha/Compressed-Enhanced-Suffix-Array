#include <stdlib.h>
#include "debug.h"

FILE* logFile;

void nop(void){
    do{}while(0);
}

void initDebug(void){
    logFile = fopen("log.txt","w");
    if(logFile==NULL){
        perror("Erro ao abrir arquivo de log");
        exit(EXIT_FAILURE);
    }
}
