#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
struct MemLns{
	void *pMem;
	int m_nSize;
	char m_szFileName[256];
	int m_nLine;
	MemLns *pNext;
	MemLns()
	{
		memset(this,0,sizeof(*this));
	}
};

class MemManager{
public:
	MemManager();
	~MemManager();
private:
	MemLns *m_pMemLnsHead;
	int m_nTotal;
public:
 	static MemManager* GetInstance();
    void Append(MemLns *pMemIns);
    void Remove(void *ptr);
    void Dump(); 
};

void *operator new(size_t size,const char*szFile, int nLine);
void operator delete(void*ptr,const char*szFile, int nLine);
void operator delete(void*ptr);
void*operator new[] (size_t size,const char*szFile,int nLine);
void operator delete[](void*ptr,const char*szFile, int nLine);
void operator delete[](void *ptr);
void PutEntry(void *ptr,int size,const char*szFile, int nLine);
void RemoveEntry(void *ptr);



#define MYNEW new(__FILE__,__LINE__)

#define MYDELETE delete
