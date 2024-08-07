#include "StringList.h"
#include "getopt.h"
#include "regex.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

enum EOptions {
  E_OPTION = 1 << 0,
  I_OPTION = 1 << 1,
  V_OPTION = 1 << 2,
  C_OPTION = 1 << 3,
  L_OPTION = 1 << 4,
  N_OPTION = 1 << 5,
  H_OPTION = 1 << 6,
  S_OPTION = 1 << 7,
  O_OPTION = 1 << 8
};

static const char* szOptionString = "ivclnhsoe:f:";
static const int nMaxBuffSize = 1024;
static BOOL bIsMultiFile = FALSE;
static DWORD dwRegexList = 0;

static struct option stLongOptions[] = {{"help", 0, NULL, 'h'}};

BOOL ReadPatterns(const char* szFilename) {
  char szBuffer[nMaxBuffSize];
  BOOL bResult = TRUE;
  if (access(szFilename, F_OK) == 0) {
    FILE* pDwFile = fopen(szFilename, "r");
    if (pDwFile) {
      while (fgets(szBuffer, nMaxBuffSize, pDwFile) != NULL) {
        AddElement(dwRegexList, szBuffer);
      }
    }
  } else {
    printf("no such file");
    bResult = FALSE;
  }
  return bResult;
}

BOOL SetOption(char cOption, int* pOptions) {
  BOOL bResult = TRUE;
  switch (cOption) {
    case 'i':
      *pOptions |= I_OPTION;
      break;
    case 'v':
      *pOptions |= V_OPTION;
      if (*pOptions & O_OPTION) {
        *pOptions ^= O_OPTION;
      }
      break;
    case 'c':
      *pOptions |= C_OPTION;
      break;
    case 'l':
      *pOptions |= L_OPTION;
      break;
    case 'n':
      *pOptions |= N_OPTION;
      break;
    case 'h':
      *pOptions |= H_OPTION;
      break;
    case 's':
      *pOptions |= S_OPTION;
      break;
    case 'o':
      if (!(*pOptions & V_OPTION)) {
        *pOptions |= O_OPTION;
      }
      break;
    case 'e':
      *pOptions |= E_OPTION;
      AddElement(dwRegexList, optarg);
      break;
    case 'f':
      *pOptions |= E_OPTION;
      bResult = ReadPatterns(optarg);
      break;
    default:
      printf("Usage: ./grep [-ivclnhso] [-e regex] [-f filename]\n");
  }

  return bResult;
}

void GetOccurrences(const char* szLine, int nProgramOptions,
                    DWORD dwOccurrencesList) {
  int nRegexCount = Size(dwRegexList);
  char* szBufLine = (char*)malloc((strlen(szLine) + 1) * sizeof(char));
  strcpy(szBufLine, szLine);
  int nResult = 0;
  while (!nResult) {
    int nMinStart = strlen(szBufLine);
    size_t nMinEnd = 0;
    for (int i = 0; i < nRegexCount; i++) {
      const char* szRegex = GetAt(dwRegexList, i);
      regex_t stRegex;
      int nICase = (nProgramOptions & I_OPTION) ? REG_ICASE : 0;
      nResult = regcomp(&stRegex, szRegex, nICase);
      if (!nResult) {
        regmatch_t stMatchPos;
        nResult = regexec(&stRegex, szBufLine, 1, &stMatchPos, 0);
        if (!nResult) {
          if (stMatchPos.rm_so < nMinStart) {
            nMinStart = stMatchPos.rm_so;
            nMinEnd = stMatchPos.rm_eo;
          }
        }
      }
    }
    if (nMinStart != (int)strlen(szBufLine)) {
      size_t nMatchSize = nMinEnd - nMinStart;
      char* szMatchPattern = (char*)malloc((nMatchSize + 1) * sizeof(char));
      memcpy(szMatchPattern, szBufLine + nMinStart, nMatchSize);
      AddElement(dwOccurrencesList, szMatchPattern);
      free(szMatchPattern);
      memmove(szBufLine, szBufLine + nMinEnd, strlen(szBufLine) - nMinEnd);
      szBufLine[strlen(szBufLine) - nMinEnd] = '\0';
      nResult = 0;
    }
  }
  free(szBufLine);
}

BOOL Matches(const char* szLine, int nProgramOptions) {
  int nRegexCount = Size(dwRegexList);
  BOOL bRetValue = FALSE;
  for (int i = 0; i < nRegexCount; i++) {
    const char* szRegex = GetAt(dwRegexList, i);
    regex_t stRegex;
    int nICase = (nProgramOptions & I_OPTION) ? REG_ICASE : 0;
    int nRetComp = regcomp(&stRegex, szRegex, nICase);
    if (!nRetComp) {
      nRetComp = regexec(&stRegex, szLine, 0, NULL, 0);
      if (!nRetComp) {
        bRetValue = TRUE;
      }
      regfree(&stRegex);
      if (bRetValue) {
        break;
      }
    }
  }
  return bRetValue;
}

void ProcessFile(const char* szFileName, int nProgramOptions) {
  char szBuffer[nMaxBuffSize];
  if (access(szFileName, F_OK) == 0) {
    FILE* pDwFile = fopen(szFileName, "r");
    int nCount = 0;
    int nLineNum = 1;
    if (pDwFile) {
      BOOL bMatches = FALSE;
      while (fgets(szBuffer, nMaxBuffSize, pDwFile) != NULL) {
        if (nProgramOptions & E_OPTION) {
          bMatches = Matches(szBuffer, nProgramOptions);
          if (nProgramOptions & V_OPTION) {
            bMatches = !bMatches;
          }
        }
        if (nProgramOptions & O_OPTION) {
          if (bMatches) {
            DWORD dwTempOccurrences = CreateStringList();
            GetOccurrences(szBuffer, nProgramOptions, dwTempOccurrences);
            int nSize = Size(dwTempOccurrences);
            for (int i = 0; i < nSize; i++) {
              if (bIsMultiFile) {
                printf("%s:%s\n", szFileName, GetAt(dwTempOccurrences, i));
              } else {
                printf("%s\n", GetAt(dwTempOccurrences, i));
              }
            }
            DeleteList(dwTempOccurrences);
          }
        }
        if (bMatches) {
          nCount++;
          if (!(nProgramOptions & O_OPTION) && !(nProgramOptions & C_OPTION) &&
              !(nProgramOptions & L_OPTION)) {
            if (nProgramOptions & N_OPTION) {
              if (bIsMultiFile) {
                printf("%s:%d:%s", szFileName, nLineNum, szBuffer);
              } else {
                printf("%d\t%s", nLineNum, szBuffer);
              }
            } else {
              if (bIsMultiFile) {
                printf("%s:%s", szFileName, szBuffer);
              } else {
                printf("%s", szBuffer);
              }
            }
          }
          if (nProgramOptions & L_OPTION) {
            break;
          }
        }
        nLineNum++;
      }
      if (nProgramOptions & C_OPTION) {
        if (bIsMultiFile) {
          printf("%s:%d\n", szFileName, nCount);
        } else {
          printf("%d\n", nCount);
        }
      }
      if (nProgramOptions & L_OPTION) {
        printf("%s\n", szFileName);
      }
      fclose(pDwFile);
    }
  } else {
    if (!(nProgramOptions & S_OPTION)) {
      printf("you provide not exists file\n");
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    printf("Grep must contain: ");
    return -1;
  }
  int nLongOptionIndex = 0;
  int nProgramOptions = 0;
  dwRegexList = CreateStringList();
  char cCurrentOption =
      GetOptLong(argc, argv, szOptionString, stLongOptions, &nLongOptionIndex);
  BOOL bResult = TRUE;
  while (cCurrentOption != -1) {
    bResult = SetOption(cCurrentOption, &nProgramOptions);
    cCurrentOption = GetOptLong(argc, argv, szOptionString, stLongOptions,
                                &nLongOptionIndex);
  }
  if (!(nProgramOptions & E_OPTION)) {
    nProgramOptions |= E_OPTION;
    AddElement(dwRegexList, argv[optind]);
    optind++;
  }
  bIsMultiFile = argc - optind > 1 ? TRUE : FALSE;
  bIsMultiFile = (nProgramOptions & H_OPTION) ? FALSE : bIsMultiFile;
  while (optind < argc && bResult) {
    const char* szFilename = argv[optind];
    ProcessFile(szFilename, nProgramOptions);
    optind++;
  }
  FreeAllLists();
  return 0;
}