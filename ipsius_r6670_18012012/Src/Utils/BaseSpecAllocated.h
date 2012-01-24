#ifndef BASESPECALLOCATED_H
#define BASESPECALLOCATED_H

#include "Platform/Platform.h"
#include "ErrorsSubsystem.h"

namespace Utils
{
    using Platform::dword;
    using Platform::byte;

    /*
        ѕредназначен дл€ наследовани€ классам использующим спецальный аллокатор.
        “ип TAllocator должен иметь методы: 
            void* alloc(size_t size);   // throw on fail
            void free(void* p);
        ƒл€ удалени€ аллокатор не нужен.
    */

    template<class TAllocator>
    class BaseSpecAllocated
    {

        enum { CIDValue = 0x105500A6 };
       
        struct BlockHeader
        {
            TAllocator *m_pAlloc;
            dword m_id;
        };


        enum { VerifyAlign = Platform::AlignAssertion<BlockHeader>::Value };
        
    protected:        

        BaseSpecAllocated()
        {
        }        

    public:        

        enum {CExtraSize = sizeof (BlockHeader)};

        virtual ~BaseSpecAllocated()
        {
        }

        static void* operator new(size_t size, TAllocator& alloc) 
        {
            BlockHeader *p = static_cast<BlockHeader*>( alloc.alloc(sizeof(BlockHeader) + size) );
            ESS_ASSERT(p != 0);

            p->m_pAlloc = &alloc;
            p->m_id = CIDValue;

            return (p + 1);
        }

        static void operator delete(void *p)
        {
            BlockHeader *pBlock = static_cast<BlockHeader*> (p);     
            --pBlock;

            ESS_ASSERT(pBlock->m_id == CIDValue);
            ESS_ASSERT(pBlock->m_pAlloc != 0);
            pBlock->m_pAlloc->free(pBlock);
        }

        static void operator delete(void *p, TAllocator&)
        {
            operator delete(p);
        }

        static void operator delete(void *p, size_t, TAllocator&)
        {
            operator delete(p);
        }

        static void *operator new[](size_t size, TAllocator& alloc)
        {
            return operator new(size, alloc);
        }

        static void operator delete[](void *p)
        {
            operator delete(p);
        }

        static void operator delete[](void *p, TAllocator&)
        {
            operator delete[](p);
        }

        /*static size_t getExtraSize()
        {
            return sizeof(BlockHeader);
        }*/

    };
} // namespace Utils

#endif




















