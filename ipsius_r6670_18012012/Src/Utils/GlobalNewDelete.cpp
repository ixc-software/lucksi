#include "stdafx.h"
#include "ProjConfigLocal.h"

#ifdef ENABLE_GLOBAL_NEW_DELETE

#include "Utils/ErrorsSubsystem.h"

#include "GlobalNewDelete.h"
#include "Platform/Platform.h"
#include "Utils/AtomicTypes.h"

/*
    TODO -- use hash table optimisation for RemoveBlock 'couse slow O(n) search
    TODO -- using mutex -- really slow sync

*/

// ------------------------------------------------------------------

namespace
{
    Utils::AtomicInt GAllocCounter;
}

namespace
{
    using namespace ProjConfig;

    class BlockList
    {
        enum { CMaxBlocks = 128 * 1024 };

        Qt::HANDLE m_threadId;
        void* m_store[CMaxBlocks];
        int m_storeCount;
        //QMutex *m_pMutex;
        //char m_buff[sizeof(QMutex)];
        Platform::Mutex m_lock;

        void AddBlock(void *p)
        {
            m_store[m_storeCount++] = p;
            if (m_storeCount >= CMaxBlocks) Abort("Out of blocks");
        }

        void RemoveBlock(void *p)
        {
            int indx = -1;

            for(int i = 0; i < m_storeCount; i++)
            {
                if (m_store[i] == p)
                {
                    indx = i;
                    break;
                }
            }

            if (indx < 0) Abort("Block not found"); 

            // remove
            if (indx != m_storeCount-1)
            {
                m_store[indx] = m_store[m_storeCount-1];
            }

            m_storeCount--;
        }

        void Abort(const char *p)
        {
            std::cerr << p << std::endl;
            std::abort();
        }

    public:

        BlockList()
        {
            // m_pMutex = new(m_buff) QMutex(QMutex::NonRecursive);

            m_threadId = 0;

            for(int i = 0; i < CMaxBlocks; i++)
            {
                m_store[i] = 0;
            }

            m_storeCount = 0;
        }

        void Add(void *p)
        {
            // m_pMutex->lock();
            m_lock.Lock();
            AddBlock(p);
            // m_pMutex->unlock();
            m_lock.Unlock();
        }

        void Remove(void *p)
        {
            // m_pMutex->lock();
            m_lock.Lock();
            RemoveBlock(p);
            // m_pMutex->unlock();
            m_lock.Unlock();
        }

        static BlockList& Instance();
    };

}

namespace
{
    void* AllocateBlock(std::size_t size)
    {
        // ESS_ASSERT(size);
        void* p = malloc(size);
        ESS_ASSERT(p && "No memory");

        GAllocCounter.Inc();
        if (GlobalNewDelete::CUseBlockList) BlockList::Instance().Add(p);

        return p;
    }

    void FreeBlock(void* p)
    {
        if (!p) return;

        free(p); 

        GAllocCounter.Dec();       
        if (GlobalNewDelete::CUseBlockList) BlockList::Instance().Remove(p);
    }

}

// ------------------------------------------------------------------

BlockList& BlockList::Instance()
{
    static BlockList list;

    return list;
}


// ------------------------------------------------------------------


void* operator new(std::size_t size)
{
    return AllocateBlock(size);
}

// ------------------------------------------------------------------

void operator delete(void* ptr)
{
    FreeBlock(ptr);
}

// ------------------------------------------------------------------

void* operator new[](std::size_t size)
{
    return AllocateBlock(size);
}

// ------------------------------------------------------------------

void operator delete[](void* ptr)
{
    FreeBlock(ptr);
}

// ------------------------------------------------------------------

int GetMemAllocatorCounter()
{
    return GAllocCounter.Get();
}

#endif
