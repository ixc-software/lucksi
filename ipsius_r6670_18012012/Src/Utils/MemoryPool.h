#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include "stdafx.h"

#include "ProjConfigLocal.h"
#include "Platform/Platform.h"
#include "ErrorsSubsystem.h"
#include "IFixedMemBlockPool.h"
#include "memorypooldetail.h"

namespace Utils
{
    using Platform::byte;
    using Platform::word;
    using Platform::dword;
    using Platform::AlignAssertion;
    using Platform::AlignAddr;
    using Platform::CMemoryAlign;
    using Platform::IntPtr;

    // ToDo: ���-�� ��������� �������������� ���� � �� �������    

    //-----------------------------------------------------------------------------


    // todo: ������, ��� ��������� ������� (���� �������� � ������, �� ��������
    // ������ catch-����� ��� ���� ��������� ������������� �������)
    ESS_TYPEDEF(NoBlocksAvailable);


    template< class TLockStrategy = DefaultThreadSafeStrategy, 
              bool CDebugMode     = CfgMemoryPool::CEnableDebug >
    class FixedMemBlockPool : public IFixedMemBlockPool
    {
        typedef typename TLockStrategy::Mutex Mutex;
        typedef typename TLockStrategy::Locker Locker;

        enum
        {
            CBHeaderID      = 0x7105A55A,  // dword!!

            CMarkerSize     = 4,
            CMarkerByte     = 0x5A
        };


        // ��������� �����
        // ������� �������� ������� ������ ���� ���������� ��� Debug � NoDebug �������,
        struct BHeader
        {
            dword m_id0;             // debug mode only
            BHeader* m_pPrev;        // debug mode only
            BHeader* m_pNext;
            dword m_id1;             // debug mode only
        };


        int m_fullBlocksize;
        int m_userBlockSize;
        int m_maxNewAlloc;
        int m_totalBlocksNumber;
        bool m_throwsExceptions;
        int m_allocatedCount;
        Mutex m_mutex;
        int m_newAllocCount; // ������� ��������� ����� ����� �������������� ������

        BHeader *m_free; // ������ ������ ���������. ���� == 0 - ������ ����.
        BHeader *m_busy; // ������ ������ �������. ���� == 0 - ������ ����.

        const bool m_useExtArray; // ����� ������������� ����������� �������������� ����� ������ ������ ����

        // Initializes a block header
        static void InitBheader(BHeader* p);

        // �������� �����
        // use if CDebugMode == true
        struct BTail
        {
            const char *m_file;
            word        m_line;
            void       *m_pMarker;
        };

        //BOOST_STATIC_ASSERT(sizeof(BTail)   %  == 0);
        //BOOST_STATIC_ASSERT(sizeof(BHeader) % AlignMultiplicity == 0);

        enum Verify
        {
            v1 = AlignAssertion<BTail>::Value,
            v2 = AlignAssertion<BHeader>::Value,
        };

        enum 
        {
            AdditionalBlockSize = sizeof(BHeader) +
                (
                    (CDebugMode) ?
                        (AlignAddr<CMarkerSize>::Value + sizeof(BTail)) : 0
                )
        };

        // Used only in debug mode
        static void InitBTail(BTail *p);

        void InitBlock(BHeader* pHeader);

        // return begin of full block
        void* CreateBlock();

        // place new block at addr and increment addr. Return begin of new block;
        BHeader* CreateBlockAt(void*& addr);

        // Used only in debug mode
        BTail* getBTail(void* userBlock);        

        // ������� ������ � ������ ���������
        void AddFrontFree(BHeader* p);

        // ������� ������ � ������ �������        
        void AddFrontBusy/*IfDebug*/(BHeader* p);

        // �������� ������ � ������ ��������� ������
        void ClearFree();

        // ����� ������� ���������
        void* PopFree();

        void RemoveFromBusy(BHeader* pHeader);      

        bool ValidHeader(BHeader* pHeader);       

        // ��������� ��������� ���������� � ����� ������        
        std::string AboutBlock(BHeader* pHeader);

        std::string AboutBlockList(BHeader* pFirst);

        void Init(int userblockSize, int maxNewAlloc);    

        void AssertAligned(void* pMemory);

        // ������ ������� ����� ����� ���������������� ������, ��� ������������
        void* WriteMarkerAtBlock(void* pBlock, int size);

        // Used only in debug mode
        // �������� ������� ����������������� �����
        bool ValidateMarker(void* pUserBlock);


    public:
        
        template<int n, int blockSize>
        struct EvalPoolSize
        {
            enum
            {
                // Additional size for n user blocks
                Value = (AdditionalBlockSize + blockSize) * n,
            };
        };

        enum {CNoLimitedNew = -1};

        /*
            int blockSize - ������ ���������� ������
            int blockCount - ����� ��������������� ������������� ������
            int maxNewAlloc - ������������ ����� ��������� ���������� ����� blockCount. ���� -1, �� �������������.
        */
        FixedMemBlockPool(int blockSize, int blocksNumber, int maxNewAlloc = CNoLimitedNew, 
                          bool throwsExceptions = true, bool autoAlignSize = true);

        /*
            int blockSize - ������ ���������� ������ (assert aligned)
            void* pMemory - ��������� ����� �� �������� ������� �������� ����� �������� blockSize (assert aligned)
            int memorySize - ������ ��������������� � ������������ ���������� ������
        */
        FixedMemBlockPool(int blockSize, void *pMemory, int memorySize, 
                          bool throwsExceptions = true, bool autoAlignSize = true);

        static int GetFullBlockSize(int blockSize);

        ~FixedMemBlockPool();

    // IFixedMemBlockPool impl:
    public:
    	
        // allocate block with size from memory pool. Size must be positive.
        void* alloc(int size, const char* file = 0, int line = 0); // can throw NoBlocksAvailable if its accepted in constructor
        
        // allocate block with max available size
        void* allocFullBlock(const char* file = 0, int line = 0);  // can throw NoBlocksAvailable if its accepted in constructor  

        void free(void* pBlock);

        int getBlockSize() const;

        int GetTotalBlocksNumber();

        // todo getFullBusyList, getNewAllocCount
    };

} // namespace Utils


#endif
