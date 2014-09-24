#include "memcheck.h"
#include "cocos2d.h"
MemManager::MemManager()
{
	m_pMemLnsHead = NULL;
	m_nTotal = 0;
}
MemManager::~MemManager()
{

}

void MemManager::Append(MemLns *pMemIns)
{
	pMemIns->pNext = m_pMemLnsHead;
	m_pMemLnsHead = pMemIns;
	m_nTotal += m_pMemLnsHead->m_nSize;
}

void MemManager::Remove(void *ptr)
{
	MemLns * pCur = m_pMemLnsHead;
	MemLns * pPrev = NULL;
	while(pCur)
	{
		if(pCur->pMem ==ptr)
		{
			if(pPrev)
			{
			   pPrev->pNext =pCur->pNext;
			}
			else
			{
			   m_pMemLnsHead =pCur->pNext;
			}
			m_nTotal-=pCur->m_nSize;
			free(pCur);
			break;
		}
		pPrev = pCur;
		pCur = pCur->pNext;
	}
	//printf("now total memory %u\n",m_nTotal);
}

void MemManager::Dump()
{
    MemLns * pp = m_pMemLnsHead;
	cocos2d::CCString info;
	int allSize = 0;
    while(pp)
    {
		info.appendWithFormat("<%u>",pp->m_nSize);
		if (pp->m_nSize > 1000)
		{
			CCLOG( "File is %s\n", pp->m_szFileName );
			CCLOG( "Size is %d\n", pp->m_nSize );
			CCLOG( "Line is %d\n", pp->m_nLine );
		}
		allSize += pp->m_nSize;
        pp = pp->pNext;
    }
	//CCLOG("%s",info.getCString());
	CCLOG("get all memory size:%u %u",allSize,m_nTotal);
}

void PutEntry(void *ptr,int size,const char*szFile, int nLine)
{
    MemLns * p = (MemLns *)(malloc(sizeof(MemLns)));
    if(p)
    {
        strcpy(p->m_szFileName,szFile);
        p->m_nLine = nLine;
        p->pMem = ptr;
        p->m_nSize = size;
        MemManager::GetInstance()->Append(p);
    }
}

void RemoveEntry(void *ptr)
{
    MemManager::GetInstance()->Remove(ptr);
}

void *operator new(size_t size,const char*szFile, int nLine)
{
    void * ptr = malloc(size);
    PutEntry(ptr,size,szFile,nLine);
    return ptr;
}

void operator delete(void *ptr)
{
    RemoveEntry(ptr);
    free(ptr);
}

void operator delete(void*ptr,const char * file, int line)
{
    RemoveEntry(ptr);
    free(ptr);
}

void*operator new[] (size_t size,const char* szFile,int nLine)
{
    void * ptr = malloc(size);
    PutEntry(ptr,size,szFile,nLine);
    return ptr;
}

void operator delete[](void *ptr)
{
    RemoveEntry(ptr);
    free(ptr);
}

void operator delete[](void*ptr,const char*szFile,int nLine)
{
    RemoveEntry(ptr);
    free(ptr);
}



MemManager m_memTracer;
MemManager*MemManager::GetInstance()
{
    return &m_memTracer;
} 