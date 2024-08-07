#ifndef CAT_H
#define CAT_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

enum EOptions {
    B_OPTION = 1,
    E_OPTION = 2,
    V_OPTION = 4,
    N_OPTION = 8,
    S_OPTION = 16,
    T_OPTION = 32
};

typedef bool BOOL;
#define TRUE 1
#define FALSE 0

#define szOptions "bevnst"
#define nBuffLen 1024

void setOption(int nOption, int* nOptions);
unsigned int getCurLineNumSize();
unsigned int Pow(unsigned int value, unsigned degree);
char charAt(unsigned int nNumber, unsigned int position);
void appendLineNum(char* szLine, unsigned int* idx);
void rowNumberLine(char* szLine, BOOL skip);
BOOL FindCharInArray(const char* szCharLine, size_t nSize, char cOrigin, unsigned int* nPosition);
void appendCharToAnother(char* szLine, size_t nLineSize, char cOrigin, char cAppendable);
size_t StrLenNewLine(const char* szLine, size_t nMaxSize);
BOOL StrReplace(char* szLine, size_t nLineSize, char cOrigin, const char* cAppendable);
void processFile(int nOptions, const char* szFileName);

#endif // CAT_H
