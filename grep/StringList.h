#ifndef STRINGLIST_H
#define STRINGLIST_H
#include "types.h"
#include <stddef.h>




DWORD CreateStringList();
void AddElement(DWORD dwList, const char* szAddString);
const char* GetAt(DWORD dwList, size_t nPosition);
BOOL RemoveAt(DWORD dwList, size_t nPosition);
BOOL DeleteList(DWORD dwList);
int Size(DWORD dwList);
BOOL Set(DWORD dwList, size_t nPosition, const char* szValue);
void FreeAllLists();
#endif