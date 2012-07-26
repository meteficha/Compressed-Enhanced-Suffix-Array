#include <stdio.h>

#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#define WHERESTR  "[function %s file %s, line %d]: "
#define WHEREARG  __FUNCTION__ ,__FILE__ , __LINE__
#define DEBUGPRINT2(...)       fprintf(logFile, __VA_ARGS__)


#ifdef MY_DEBUG
    #define printDebug(_fmt, ...)  DEBUGPRINT2(WHERESTR _fmt, WHEREARG, __VA_ARGS__); fflush(stdout)
#else
    #define printDebug(_fmt, ...) nop()
#endif

extern FILE* logFile;
void nop(void);
void initDebug(void);

#endif // DEBUG_H_INCLUDED
