#ifndef MEMORYPOOLBODY_H
#define MEMORYPOOLBODY_H

#include "stdafx.h"
#include "MemoryPool.h"

namespace Utils
{    
    using Platform::IntPtr;

    template< class TLockStrategy, bool CDebugMode>
    void FixedMemBlockPool<TLockStrategy, CDebugMode>::InitBheader( BHeader* p )
    {
        p->m_pNext = 0;

        if (CDebugMode)
        {
            p->m_pPrev = 0;
            p->m_id0 = CBHeaderID;
            p->m_id1 = CBHeaderID;
        }
    }    

    template< class TLockStrategy, bool CDebugMode>
    void FixedMemBlockPool<TLockStrategy, CDebugMode>::InitBTail( BTail *p )
    {
        // ESS_ASSERT(CDebugMode);
        p->m_file        = 0;
        p->m_line        = 0;
        p->m_pMarker     = 0;
    }

    template< class TLockStrategy, bool CDebugMode>
    void FixedMemBlockPool<TLockStrategy, CDebugMode>::InitBlock( BHeader* pHeader )
    {
        InitBheader(pHeader);

        if (CDebugMode)
            InitBTail( getBTail(pHeader + 1) );
    }

    template< class TLockStrategy, bool CDebugMode>
    void* FixedMemBlockPool<TLockStrategy, CDebugMode>::CreateBlock()
    {
        void *ret = new byte[m_fullBlocksize];
        InitBlock(static_cast<BHeader*>(ret));
        return ret;
    }

    template< class TLockStrategy, bool CDebugMode>
    typename FixedMemBlockPool<TLockStrategy, CDebugMode>::BHeader* 
        FixedMemBlockPool<TLockStrategy, CDebugMode>::CreateBlockAt( void*& addr )
    {
        byte    *pBlockByte = new(addr) byte[m_fullBlocksize];

        BHeader *pBlock = reinterpret_cast<BHeader*>(pBlockByte);
        InitBlock(pBlock);

        pBlockByte += m_fullBlocksize;
        addr = pBlockByte;

        return pBlock;
    }

    template< class TLockStrategy, bool CDebugMode>
    typename FixedMemBlockPool<TLockStrategy, CDebugMode>::BTail*
        FixedMemBlockPool<TLockStrategy, CDebugMode>::getBTail( void* userBlock )
    {
        // ESS_ASSERT(CDebugMode); !
        userBlock = static_cast<byte*>(userBlock) + // pointer to the block
            m_userBlockSize +               // fixed (maximum) block size
            AlignAddr<CMarkerSize>::Value;  // size of the marker at the end of the block

        return static_cast<BTail*>(userBlock);
    }

    template< class TLockStrategy, bool CDebugMode>
    void FixedMemBlockPool<TLockStrategy, CDebugMode>::AddFrontFree( BHeader* p )
    {
        p->m_pNext = m_free;
        m_free = p;
    }

    template< class TLockStrategy, bool CDebugMode>
    void FixedMemBlockPool<TLockStrategy, CDebugMode>::AddFrontBusy( BHeader* p )
    {
        //if (!CDebugMode) return; 
        p->m_pNext = m_busy;  

        if (m_busy != 0)
            m_busy->m_pPrev = p;

        m_busy = p;
    }

    template< class TLockStrategy, bool CDebugMode>
    void FixedMemBlockPool<TLockStrategy, CDebugMode>::ClearFree()
    {
        while (m_free != 0)
        {
            byte* p = static_cast<byte*>( PopFree() );
            delete[] p;
        }
    }


    template< class TLockStrategy, bool CDebugMode>
    void* FixedMemBlockPool<TLockStrategy, CDebugMode>::PopFree()
    {
        BHeader *p = m_free;                
        m_free = p->m_pNext;                
        return p;
    }

    template< class TLockStrategy, bool CDebugMode>
    void FixedMemBlockPool<TLockStrategy, CDebugMode>::RemoveFromBusy( BHeader* pHeader )
    {
        // удаление из списка зан€тых (схлопнуть список)
        BHeader* prevBlock = pHeader->m_pPrev;
        BHeader* nextBlock = pHeader->m_pNext;

        InitBlock(pHeader);

        if (prevBlock != 0)
            prevBlock->m_pNext = nextBlock;
        else
            m_busy = nextBlock; // удал€емый блок - вершина

        if (nextBlock != 0)
            nextBlock->m_pPrev = prevBlock;
    }

    template< class TLockStrategy, bool CDebugMode>
    bool FixedMemBlockPool<TLockStrategy, CDebugMode>::ValidHeader( BHeader* pHeader )
    {
        return (pHeader->m_id0 == CBHeaderID) && (pHeader->m_id1 == CBHeaderID);
    }

    template< class TLockStrategy, bool CDebugMode>
    std::string FixedMemBlockPool<TLockStrategy, CDebugMode>::AboutBlock( BHeader* pHeader )
    {
        ESS_ASSERT(CDebugMode); // использование этой фции недопустимо в неотладочном режиме
        std::ostringstream ss;
        // todo class name through typeid
        ss << "Info about memory block as allocated by FixedMemBlockPool\nwith fixed user block size = " 
            << m_userBlockSize << " at address 0x" ;//<< std::hex << (dword)(pHeader + 1) << ":\n";
        ss.setf(std::ios::uppercase);
        ss << std::hex << (dword)(pHeader + 1) << ":\n";

        ss << "\tHeader record is " << (ValidHeader(pHeader) ? "valid" : "damaged") << ".\n";

        // TODO ”простить код:
        ss << "\tTail record is ";
        void * pUserBlock = pHeader + 1;
        BTail* pTail = getBTail(pUserBlock);

        ss << "valid.\n\tAllocation point: ";
        if (pTail->m_file !=0) ss << "File - " 
            << getBTail(pUserBlock)->m_file << ", line - " << getBTail(pUserBlock)->m_line << ".\n";
        else
            ss << "dosn`t write.\n";


        return ss.str();
    }

    template< class TLockStrategy, bool CDebugMode>
    std::string FixedMemBlockPool<TLockStrategy, CDebugMode>::AboutBlockList( BHeader* pFirst )
    {
        ESS_ASSERT(CDebugMode);
        ESS_ASSERT(pFirst);
        std::string rez;

        while(pFirst)
        {
            rez += AboutBlock(pFirst);
            if ( ValidHeader(pFirst) )
                pFirst = pFirst->m_pNext;
            else
                pFirst = 0;
        }
        return rez;
    }

    template< class TLockStrategy, bool CDebugMode>
    void FixedMemBlockPool<TLockStrategy, CDebugMode>::Init( int userblockSize, int maxNewAlloc )
    {
        ESS_ASSERT(userblockSize >= 0);
        m_userBlockSize = userblockSize;
        m_fullBlocksize = userblockSize + AdditionalBlockSize;
        m_maxNewAlloc = maxNewAlloc;
        m_newAllocCount = 0;
        m_free = 0;
        m_busy = 0;
    }

    template< class TLockStrategy, bool CDebugMode>
    void FixedMemBlockPool<TLockStrategy, CDebugMode>::AssertAligned( void* pMemory )
    {
        if (CDebugMode)
            ESS_ASSERT( IntPtr(pMemory).ValueIsAligned() );
    }

    template< class TLockStrategy, bool CDebugMode>
    void* FixedMemBlockPool<TLockStrategy, CDebugMode>::WriteMarkerAtBlock( void* pBlock, int size )
    {
        // Ѕлок может быть нулевого размера!
        //if (size == 0) return 0;

        byte *pMarker = static_cast<byte*>(pBlock) + size;
        void *ret = pMarker;

        for (int i = 0; i < CMarkerSize; ++i)
        {
            *pMarker++ = CMarkerByte;
        }

        return ret;
    }

    template< class TLockStrategy, bool CDebugMode>
    bool FixedMemBlockPool<TLockStrategy, CDebugMode>::ValidateMarker( void* pUserBlock )
    {
        BTail *pBlockTail = getBTail(pUserBlock);
        byte  *pMarker    = static_cast<byte*>(pBlockTail->m_pMarker);

        // ѕроверка адреса маркера
        if ((pMarker <  pUserBlock) ||
            (pMarker >= IntPtr(pBlockTail).ToPtr<byte>()))
        {
            return false;
        }

        // ѕроверка маркера
        for (int i = 0; i < CMarkerSize; ++i)
        {
            // ¬озврат, если маркер поврежден
            if (*pMarker++ != CMarkerByte)
                return false;
        }

        return true;
    }

    // ------------------------------------------------------------------------

    /*
        int blockSize - размер выдел€емых блоков
        int blockCount - число преаллоцируемых конструктором блоков
        int maxNewAlloc - максимальное число аллокаций допустимых сверх blockCount. ≈сли -1, то неограниченно.
    */
    template< class TLockStrategy, bool CDebugMode>
    FixedMemBlockPool<TLockStrategy, CDebugMode>::FixedMemBlockPool( int blockSize, int blocksNumber, int maxNewAlloc, 
        bool throwsExceptions, bool autoAlignSize) : 
        m_throwsExceptions(throwsExceptions),
        m_allocatedCount(0), 
        m_useExtArray(false)
    {
        ESS_ASSERT(blockSize >= 0);
        
        if (maxNewAlloc == 0)
            ESS_ASSERT(blocksNumber >  0);
        else
            ESS_ASSERT(blocksNumber >= 0);

        ESS_ASSERT((maxNewAlloc >= 0) || (maxNewAlloc == CNoLimitedNew));

        if (autoAlignSize) blockSize = Platform::AlignRuntime::Calc(blockSize);
        ESS_ASSERT( IntPtr(blockSize).ValueIsAligned() );

        Init(blockSize, maxNewAlloc);

        m_free = static_cast<BHeader*>(CreateBlock());
        for(int i = 1; i < blocksNumber; ++i)
        {
            AddFrontFree( static_cast<BHeader*>(CreateBlock()) );
        }

        m_totalBlocksNumber = (m_maxNewAlloc == CNoLimitedNew ? CNoLimitedNew : blocksNumber + m_maxNewAlloc);
    }

    // ------------------------------------------------------------------------

    /*
        int blockSize - размер выдел€емых блоков (assert aligned)
        void* pMemory - начальный адрес из которого следует выдел€ть блоки размером blockSize (assert aligned)
        int memorySize - размер предоставл€емой в распор€жение аллокатору пам€ти
    */
    template< class TLockStrategy, bool CDebugMode>
    FixedMemBlockPool<TLockStrategy, CDebugMode>::FixedMemBlockPool( int blockSize, void *pMemory, int memorySize, 
                                                                     bool throwsExceptions, bool autoAlignSize) : 
        m_throwsExceptions(throwsExceptions),
        m_allocatedCount(0), 
        m_useExtArray(true)
    {
        ESS_ASSERT(blockSize >= 0);
        ESS_ASSERT(memorySize > 0);

        if (autoAlignSize) blockSize = Platform::AlignRuntime::Calc(blockSize);
        ESS_ASSERT( IntPtr(blockSize).ValueIsAligned() );

        ESS_ASSERT(IntPtr(pMemory).ValueIsAligned() == true);

        Init(blockSize, 0);

        int m_totalBlocksNumber = memorySize / m_fullBlocksize;
        ESS_ASSERT(m_totalBlocksNumber > 0);

        m_free = static_cast<BHeader*>( CreateBlockAt(pMemory) );
        for (int i = 1; i < m_totalBlocksNumber; ++i)
        {
            //ESS_ASSERT(pMemory < )
            AddFrontFree( static_cast<BHeader*>( CreateBlockAt(pMemory) ) );
        }
    }

    // ------------------------------------------------------------------------

    // destructor
    template< class TLockStrategy, bool CDebugMode>
    FixedMemBlockPool<TLockStrategy, CDebugMode>::~FixedMemBlockPool()
    {
        // ESS_ASSERT(m_allocatedCount == 0);

        if (m_allocatedCount != 0)
        {
            std::ostringstream ss;
            ss << "~FixedMemBlockPool allocated " << m_allocatedCount;
            ESS_HALT(ss.str());
        }

        if (CDebugMode)
        {
            if (m_busy != 0)
            {
                ESS_HALT( "Some blocks are not free!\n" + AboutBlockList(m_busy) );
            }                
        }

        if (!m_useExtArray)
            ClearFree();
    }

    //-------------------------------------------------------------------------------------

    template< class TLockStrategy, bool CDebugMode>
    void* FixedMemBlockPool<TLockStrategy, CDebugMode>::allocFullBlock(const char* file, int line)
    {
        return alloc(getBlockSize(), file, line);
    }

    //-------------------------------------------------------------------------------------

    template< class TLockStrategy, bool CDebugMode>
    void* FixedMemBlockPool<TLockStrategy, CDebugMode>::alloc( int size, const char* file /*= 0*/, int line /*= 0*/ )
    {        
        if (CDebugMode)
        {
            ESS_ASSERT((size > 0) && (size <= m_userBlockSize));
            // assert aligned size???
        }

        Locker locker(m_mutex);


        void* pBlock;

        if (m_free != 0)
        {
            pBlock = PopFree();
        }
        else
        {
            if (m_maxNewAlloc != CNoLimitedNew) // new blocks are limited
            {
                if (++m_newAllocCount > m_maxNewAlloc)
                {
                    if (m_throwsExceptions) ESS_THROW(NoBlocksAvailable);
                    else return 0;
                }
            }

            pBlock = CreateBlock();
        }


        if (CDebugMode) AddFrontBusy(static_cast<BHeader*>(pBlock));

        pBlock = static_cast<BHeader*>(pBlock) + 1;

        if (CDebugMode)
        {
            BTail *pTail         = getBTail(pBlock);
            pTail->m_file        = file;
            pTail->m_line        = line;
            pTail->m_pMarker     = WriteMarkerAtBlock(pBlock, size);
        }

        ++m_allocatedCount;

        return pBlock;
    }

    //-------------------------------------------------------------------------------------

    template< class TLockStrategy, bool CDebugMode>
    void FixedMemBlockPool<TLockStrategy, CDebugMode>::free( void* pBlock )
    {
        AssertAligned(pBlock);

        Locker locker(m_mutex);

        // get real head of block
        BHeader* pHeader = static_cast<BHeader*>(pBlock) - 1;

        // validate block
        if (CDebugMode)
        {
            if (!ValidHeader(pHeader) || !ValidateMarker(pBlock))
            {
                ESS_HALT( AboutBlock(pHeader) );
            }                

            // удал€ем из списка зан€тых
            RemoveFromBusy(pHeader);
        }

        // подшиваем к списку свободных
        AddFrontFree(pHeader);

        --m_allocatedCount;
    }

    template< class TLockStrategy, bool CDebugMode>
    int FixedMemBlockPool<TLockStrategy, CDebugMode>::getBlockSize() const
    {
        return m_userBlockSize;
    }

    template< class TLockStrategy, bool CDebugMode>
    int FixedMemBlockPool<TLockStrategy, CDebugMode>::GetTotalBlocksNumber()
    {
        return m_totalBlocksNumber;
    }

    template< class TLockStrategy, bool CDebugMode>
    int FixedMemBlockPool<TLockStrategy, CDebugMode>::GetFullBlockSize( int blockSize )
    {
        //  опи€ AdditionalBlockSize!!!
        int headerSize = sizeof(BHeader) +
            (
            (CDebugMode) ? (AlignAddr<CMarkerSize>::Value + sizeof(BTail)) : 0
            );

        return blockSize + headerSize;
    }
} // namespace Utils


#endif
