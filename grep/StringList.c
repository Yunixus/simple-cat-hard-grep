#include "StringList.h"

#include "memory.h"
#include "stdlib.h"

typedef struct Node {
  char* szValue;
  struct Node* pNext;
} Node;

static DWORD dwCurrentSize = 0;
static struct Node* pTable = NULL;
size_t* pNodeSizes = NULL;
static DWORD dwCurrentTableSize = 0;
static unsigned nCountNewElem = 5;

typedef struct UnusedNode {
  DWORD dwUnused;
  struct UnusedNode* pNext;
} UnusedNode;

static UnusedNode* pUnusedLists = NULL;

BOOL GetUnused(DWORD* pDwUnused) {
  if (pUnusedLists == NULL) {
    return FALSE;
  } else {
    *pDwUnused = pUnusedLists->dwUnused;
    UnusedNode* pTempMem = pUnusedLists;
    pUnusedLists = pUnusedLists->pNext;
    free(pTempMem);
    return TRUE;
  }
}

void PutUnused(DWORD dwUnused) {
  if (pUnusedLists == NULL) {
    pUnusedLists = malloc(sizeof(UnusedNode));
    pUnusedLists->dwUnused = dwUnused;
  } else {
    UnusedNode* pCurrent = pUnusedLists;
    while (pCurrent->pNext != NULL) {
      pCurrent = pCurrent->pNext;
    }
    UnusedNode* pNewNode = malloc(sizeof(UnusedNode));
    pNewNode->dwUnused = dwUnused;
    pCurrent->pNext = pNewNode;
  }
}

DWORD CreateStringList() {
  DWORD dwNewList = 0;
  BOOL bHaveUnused = GetUnused(&dwNewList);
  if (bHaveUnused) {
    return dwNewList;
  }
  if (pTable == NULL) {
    pTable = malloc(sizeof(Node) * nCountNewElem);
    pNodeSizes = malloc(sizeof(size_t) * nCountNewElem);
    for (DWORD i = 0; i < nCountNewElem; i++) {
      pNodeSizes[i] = 0;
    }
    dwCurrentTableSize = nCountNewElem;
  }
  if (dwCurrentSize >= dwCurrentTableSize) {
    Node* pTempMem =
        realloc(pTable, (dwCurrentTableSize + nCountNewElem) * sizeof(Node));
    if (pTempMem) {
      pTable = pTempMem;
    }
    size_t* pTempSizes = realloc(
        pNodeSizes, (dwCurrentTableSize + nCountNewElem) * sizeof(size_t));
    if (pTempSizes) {
      pNodeSizes = pTempSizes;
      for (DWORD i = dwCurrentTableSize; i < dwCurrentTableSize + nCountNewElem;
           i++) {
        pNodeSizes[i] = 0;
      }
    }
    if (pTempSizes && pTempMem) {
      dwCurrentTableSize += nCountNewElem;
    }
  }
  dwNewList = dwCurrentSize++;
  return dwNewList;
}

void AddElement(DWORD dwList, const char* szAddString) {
  if (dwList > dwCurrentSize) {
    return;
  }
  Node* pRoot = &pTable[dwList];
  if (pNodeSizes[dwList] == 0) {
    pRoot->szValue = malloc((strlen(szAddString) + 1) * sizeof(char));
    strcpy(pRoot->szValue, szAddString);
  } else {
    Node* pCurrItem = pRoot;
    while (pCurrItem->pNext != NULL) {
      pCurrItem = pCurrItem->pNext;
    }
    Node* pNewItem = malloc(sizeof(Node));
    pNewItem->szValue = malloc((strlen(szAddString) + 1) * sizeof(char));
    strcpy(pNewItem->szValue, szAddString);
    pCurrItem->pNext = pNewItem;
  }
  pNodeSizes[dwList]++;
}

const char* GetAt(DWORD dwList, size_t position) {
  if (dwList > dwCurrentSize) {
    return "";
  }
  if (position > pNodeSizes[dwList]) {
    return "";
  }
  int nIter = 0;
  Node* pCurrent = &pTable[dwList];
  while (nIter < (int)position) {
    pCurrent = pCurrent->pNext;
    nIter++;
  }
  return pCurrent->szValue;
}

int Size(DWORD dwList) { return (int)pNodeSizes[dwList]; }

BOOL Set(DWORD dwList, size_t nPosition, const char* szValue) {
  if (dwList > dwCurrentSize) {
    return FALSE;
  }
  if (nPosition > pNodeSizes[dwList]) {
    return FALSE;
  }
  int nIter = 0;
  Node* pCurrent = &pTable[dwList];
  while (nIter < (int)nPosition) {
    pCurrent = pCurrent->pNext;
    nIter++;
  }
  if (pCurrent->szValue != NULL) {
    free(pCurrent->szValue);
    pCurrent->szValue = NULL;
  }
  pCurrent->szValue = malloc(sizeof(char) * (strlen(szValue) + 1));
  strcpy(pCurrent->szValue, szValue);
  return TRUE;
}

BOOL RemoveAt(DWORD dwList, size_t nPosition) {
  if (dwList > dwCurrentSize) {
    return FALSE;
  }
  if (nPosition > pNodeSizes[dwList]) {
    return FALSE;
  }
  if (nPosition == 0) {
    Node* pDelMem = pTable[dwList].pNext;
    if (pTable[dwList].szValue != NULL) {
      free(pTable[dwList].szValue);
      pTable[dwList].szValue = NULL;
    }
    pTable[dwList].szValue = pDelMem->szValue;
    pTable[dwList].pNext = pDelMem->pNext;
    free(pDelMem);
  }
  int nIter = 0;
  Node* pCurrent = &pTable[dwList];
  while (nIter != (int)nPosition - 1) {
    pCurrent = pCurrent->pNext;
    nIter++;
  }
  if (pCurrent->pNext->szValue != NULL) {
    free(pCurrent->pNext->szValue);
    pCurrent->pNext->szValue = NULL;
  }
  Node* delMem = pCurrent->pNext;
  pCurrent->pNext = pCurrent->pNext->pNext;
  free(delMem);
  return TRUE;
}

BOOL DeleteList(DWORD dwList) {
  if (dwList > dwCurrentSize) {
    return FALSE;
  }
  Node* Root = &pTable[dwList];
  if (Root->szValue != NULL) {
    free(Root->szValue);
    Root->szValue = NULL;
  }
  Node* pCurrent = Root->pNext;
  Root->pNext = NULL;
  while (pCurrent != NULL) {
    if (pCurrent->szValue != NULL) {
      free(pCurrent->szValue);
      pCurrent->szValue = NULL;
    }
    Node* pDelMem = pCurrent;
    pCurrent = pCurrent->pNext;
    pDelMem->pNext = NULL;
    free(pDelMem);
  }
  PutUnused(dwList);
  pNodeSizes[dwList] = 0;
  return TRUE;
}

void FreeAllLists() {
  if (pTable != NULL) {
    for (DWORD i = 0; i < dwCurrentSize; i++) {
      DeleteList(i);
    }
    free(pTable);
    free(pNodeSizes);
  }
  while (pUnusedLists != NULL) {
    UnusedNode* pDelMem = pUnusedLists;
    pUnusedLists = pUnusedLists->pNext;
    free(pDelMem);
  }
}