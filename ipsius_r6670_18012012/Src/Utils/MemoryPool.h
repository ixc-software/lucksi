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

    // ToDo:  ак-то исключить неиспользуемые пол€ в не отладки    

    //-----------------------------------------------------------------------------


    // todo: ”знать, как правильно сделать (если объ€вить в классе, то придетс€
    // делать catch-блоки дл€ всех вариантов использовани€ шаблона)
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


        // «аголовок блока
        // Ќаверно логичней сделать разные типы заголовков дл€ Debug и NoDebug режимов,
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
        int m_newAllocCount; // счетчик аллокаций сверх числа предвыделенных блоков

        BHeader *m_free; // начало списка свободных. ≈сли == 0 - список пуст.
        BHeader *m_busy; // начало списка зан€тых. ≈сли == 0 - список пуст.

        const bool m_useExtArray; // режим использовани€ переданного фиксированного блока пам€ти вместо кучи

        // Initializes a block header
        static void InitBheader(BHeader* p);

        //  онцевик блока
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

        // подшить сверху к списку свободных
        void AddFrontFree(BHeader* p);

        // подшить сверху к списку зан€тых        
        void AddFrontBusy/*IfDebug*/(BHeader* p);

        // зачистка пам€ти в списке свободных блоков
        void ClearFree();

        // сн€ть верхний свободный
        void* PopFree();

        void RemoveFromBusy(BHeader* pHeader);      

        bool ValidHeader(BHeader* pHeader);       

        // ‘ормирует строковую информацию о блоке пам€ти        
        std::string AboutBlock(BHeader* pHeader);

        std::string AboutBlockList(BHeader* pFirst);

        void Init(int userblockSize, int maxNewAlloc);    

        void AssertAligned(void* pMemory);

        // запись маркера сразу после пользовательских данных, без выравнивани€
        void* WriteMarkerAtBlock(void* pBlock, int size);

        // Used only in debug mode
        // проверка маркера пользовательского блока
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
            int blockSize - размер выдел€емых блоков
            int blockCount - число преаллоцируемых конструктором блоков
            int maxNewAlloc - максимальное число аллокаций допустимых сверх blockCount. ≈сли -1, то неограниченно.
        */
        FixedMemBlockPool(int blockSize, int blocksNumber, int maxNewAlloc = CNoLimitedNew, 
                          bool throwsExceptions = true, bool autoAlignSize = true);

        /*
            int blockSize - размер выдел€емых блоков (assert aligned)
            void* pMemory - начальный адрес из которого следует выдел€ть блоки размером blockSize (assert aligned)
            int memorySize - размер предоставл€емой в распор€жение аллокатору пам€ти
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
