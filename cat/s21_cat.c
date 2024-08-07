#include "getopt.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

enum EOptions {
  B_OPTION = 1,
  E_OPTION = 2,
  V_OPTION = 4,
  N_OPTION = 8,
  S_OPTION = 16,
  T_OPTION = 32
};

typedef char BOOL;
#define TRUE 1
#define FALSE 0

static const char szOptions[] = "bevnst";
static const unsigned short nBuffLen = 1024;
unsigned int nLineNum = 1;

void setOption(int nOption, int* nOptions) {
  switch (nOption) {
    case 'n':
      *nOptions |= N_OPTION;
      break;
    case 'b':
      *nOptions |= B_OPTION;
      break;
    case 'e':
      *nOptions |= E_OPTION;
      break;
    case 'v':
      *nOptions |= V_OPTION;
      break;
    case 's':
      *nOptions |= S_OPTION;
      break;
    case 't':
      *nOptions |= T_OPTION;
      break;
    default:
      break;
  }
}

unsigned int getCurLineNumSize() {
  int nNum = 0;
  unsigned int nLine = nLineNum;
  while (nLine != 0) {
    nNum++;
    nLine /= 10;
  }
  return nNum;
}

unsigned int Pow(unsigned int value, unsigned degree) {
  if (degree == 0) {
    return 1;
  }
  unsigned int retValue = value;
  degree--;
  while (degree != 0) {
    retValue *= value;
    degree--;
  }
  return retValue;
}

char charAt(unsigned int nNumber, unsigned int position) {
  nNumber = nNumber / Pow(10, position);
  return (nNumber % 10) + '0';
}

void appendLineNum(char* szLine, unsigned int* idx) {
  unsigned int nSize = getCurLineNumSize();
  while (nSize > 0) {
    char nCurCh = charAt(nLineNum, nSize - 1);
    szLine[(*idx)++] = nCurCh;
    nSize--;
  }
}

void rowNumberLine(char* szLine, BOOL skip) {
  unsigned nLineSize = strlen(szLine);
  skip = (szLine[0] != '\n' && szLine[0] != '$') ? FALSE : skip;
  char* szBufLine = NULL;
  unsigned int nCurIdx = 0;
  if (skip != TRUE) {
    unsigned int nSpacesSize =
        6 - getCurLineNumSize() > 0 ? 6 - getCurLineNumSize() : 0;
    nLineSize += getCurLineNumSize() + nSpacesSize + 2;
    szBufLine = (char*)malloc(nLineSize * sizeof(char));
    memset(szBufLine, ' ', nSpacesSize);
    nCurIdx = nSpacesSize;
    appendLineNum(szBufLine, &nCurIdx);
    nLineNum++;
  } else {
    nLineSize += 8;
    szBufLine = (char*)malloc(nLineSize * sizeof(char));
    memset(szBufLine, ' ', 6);
    nCurIdx = 6;
  }
  szBufLine[nCurIdx] = '\t';
  strcat(szBufLine, szLine);
  strcpy(szLine, szBufLine);
  free(szBufLine);
}

BOOL FindCharInArray(const char* szCharLine, size_t nSize, char cOrigin,
                     unsigned int* nPosition) {
  for (size_t i = 0; i < nSize; i++) {
    if (szCharLine[i] == cOrigin) {
      *nPosition = i;
      return TRUE;
    }
  }
  return FALSE;
}

void appendCharToAnother(char* szLine, size_t nLineSize, char cOrigin,
                         char cAppendable) {
  unsigned int index = 0;
  BOOL bContains = FindCharInArray(szLine, nLineSize, cOrigin, &index);
  if (bContains) {
    char cCurCh = cAppendable;
    char cCurNext = cOrigin;
    while (index < nLineSize + 2) {
      szLine[index++] = cCurCh;
      cCurCh = cCurNext;
      cCurNext = szLine[index];
    }
  }
}

size_t StrLenNewLine(const char* szLine, size_t nMaxSize) {
  size_t index = 0;
  while (index != nMaxSize) {
    if (szLine[index++] == '\n') {
      break;
    }
  }
  return index;
}

BOOL StrReplace(char* szLine, size_t nLineSize, char cOrigin,
                const char* cAppendable) {
  unsigned int index = 0;
  BOOL bContains = FindCharInArray(szLine, nLineSize, cOrigin, &index);
  if (bContains) {
    size_t nLength = nLineSize + strlen(cAppendable);
    char* szBufLine = malloc((nLength) * sizeof(char));
    memcpy(szBufLine, szLine, index);
    memcpy(szBufLine + index, cAppendable, strlen(cAppendable));
    memcpy(szBufLine + index + strlen(cAppendable), szLine + index + 1,
           nLineSize - index - 1);
    memcpy(szLine, szBufLine, nLength);
    free(szBufLine);
    return TRUE;
  } else {
    return FALSE;
  }
}

void processFile(int nOptions, const char* szFileName) {
  char szBuffer[nBuffLen];
  if (access(szFileName, F_OK) == 0) {
    FILE* pDwFile = fopen(szFileName, "r");
    if (pDwFile) {
      BOOL bIsNewLine = FALSE;
      while ((fgets(szBuffer, nBuffLen, pDwFile)) != NULL) {
        if (nOptions & S_OPTION) {
          if (szBuffer[0] == '\n') {
            if (bIsNewLine) {
              continue;
            }
            bIsNewLine = TRUE;
          } else {
            bIsNewLine = FALSE;
          }
        }
        if (nOptions & T_OPTION) {
          BOOL bContains = TRUE;
          while (bContains) {
            size_t nLineSize = StrLenNewLine(szBuffer, nBuffLen);
            bContains = StrReplace(szBuffer, nLineSize, '\t', "^I");
          }
        }
        if (nOptions & E_OPTION) {
          size_t nLineSize = StrLenNewLine(szBuffer, nBuffLen);
          appendCharToAnother(szBuffer, nLineSize, '\n', '$');
        }
        if (nOptions & B_OPTION) {
          rowNumberLine(szBuffer, TRUE);
        } else if (nOptions & N_OPTION) {
          rowNumberLine(szBuffer, FALSE);
        }
        size_t nLineSize = StrLenNewLine(szBuffer, nBuffLen);
        for (size_t i = 0; i < nLineSize; i++) {
          putchar(szBuffer[i]);
        }
      }
      fclose(pDwFile);
    }
  } else {
    printf("no such file\n");
  }
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    printf("no option\n");
    return 0;
  }

  int nProgramOptions = 0;
  int nCurOption = getopt(argc, argv, szOptions);
  while (nCurOption != -1) {
    setOption(nCurOption, &nProgramOptions);
    nCurOption = getopt(argc, argv, szOptions);
  }
  while (optind < argc) {
    const char* fileName = argv[optind];
    processFile(nProgramOptions, fileName);
    nLineNum = 1;
    optind = optind + 1;
  }
  return 0;
}