//-----------------------------------------------------------------------------
//
//                                                            CEnableDebug
//                                                          true      false
// - InitBheader()                                              
//      установка значения BHeader::pNext                     +         +
//      установка значений debug-полей BHeader                +         -
//
// - InitBTail():
//      установка значений структуры хвоста блока             +        n/a
//
// - InitBlock():
//      вызов InitBheader()                                   +         +
//      вызов InitBTail()                                     +         -
//
// - CreateBlock():
//      вызов InitBlock()                                     +         +
//
// - CreateBlockAt():
//      вызов InitBlock()                                     +         +
//
// - getBTail()
//      возвращает указатель на хвост блока                   +        n/a
//
// - AddFrontFree()
//      удаляет блок из списка свободных                      +         +
//
// - AddFrontBusy()
//      добавляет блок к списку занятых                       +        n/a
//
// - ClearFree()
//      ...                                                   +         +
//
// - PopFree()
//      ...                                                   +         +
//
// - RemoveFromBusy()
//      вызывает InitBlock                                    +         -
//
// - ValidHeader()
//      возвращает true, если заголовок корректный            +        n/a
//
// - Init()
//      проверяется размер выделяемого блока (userBlockSize)  +         +
//
// - AssertAligned()
//      проверяет выравненность адреса                        +         -
//
// - WriteMarkerAtBlock()
//      if (size == 0) return 0; !!!!!!                       +        n/a
//
// - ValidateMarker()
//      возвращает 0, если маркер корректный                  +        n/a
//
// - FixedMemBlockPool():
//      проверка значений аргументов                          +         +
//      вызывает CreateBlock()                                +         +
//      вызывает AddFrontFree()                               +         +
//
// - FixedMemBlockPool() для предвыделенного блока памяти:
//      проверка значений аргументов                          +         +
//      вызывает CreateBlockAt()                              +         +
//      вызывает AddFrontFree()                               +         +
//
// - ~FixedMemBlockPool():
//      проверка очереди занятых блоков (m_busy == 0)         +         -
//      вызывает ClearFree(), если память выделялась объектом +         +
//
// - alloc()
//      проверка аргументов                                   +         -
//      вызов PopFree()                                       +         +
//      проверка m_maxNewAlloc                                +         +
//      вызов CreateBlock()                                   +         +
//      вызов AddFrontBusy()                                  +         -
//      добавление "хвоста" блока (+вызов getBTail())         +         -
//
// - free()
//      вызывает AssertAligned()                              +         +
//      проверка корректности заголовка и маркера             +         -
//      исключение блока из списка занятых                    +         -
// 
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/MemoryPool.h"
#include "Utils/Random.h"
#include "MemoryPoolTests.h"

namespace
{
    class FixedMemBlockPoolTest
    {
        static const int CBlockSize          = 256;
        static const int CUserBlockSize      = 10;
        static const int CBlocksNumber       = 10;
        static const int CExtraBlocksNumber  = 3;
        static const int CIterationsNumber   = 1024;

    public:
        static void Test();
        static void UsingSimulation();
    };

}

namespace
{

    using namespace std;
    using namespace Platform;
    using namespace Utils;

    using boost::scoped_ptr;

    using ESS::ExceptionHookRethrow;
    using ESS::HookRethrow;


    class MemoryBlock
    {
        void        *m_blockAddress;    // address returned by the alloc() function
        int          m_blockSize;       // size of the block
        byte         m_xor;             // xor of the block's data


    public:
        MemoryBlock(void *blockAddress, int blockSize) :
            m_blockAddress(blockAddress), m_blockSize(blockSize), m_xor(CalculateXor(blockAddress, blockSize))
        {
            ESS_ASSERT(blockAddress != 0);
            ESS_ASSERT(m_blockSize  >= 0);
        }


        //---------------------------------------------------------------------


        void* getBlockAddress() const { return m_blockAddress; }
        int   getBlockSize() const    { return m_blockSize; }
        byte  getXor() const          { return m_xor; }


        //---------------------------------------------------------------------


        static byte CalculateXor(void *pData, int length)
        {
            ESS_ASSERT(pData  != 0);
            ESS_ASSERT(length >= 0);

            byte *pCursor = IntPtr(pData).ToPtr<byte>();

            /*
            byte xor = 0;

            while (length--)
            {
                xor ^= *pCursor++;
            }

            return xor;
            */
            byte xorByte = 0;

            while (length--)
            {
                xorByte ^= *pCursor++;
            }

            return xorByte;
        }

    };


    //-------------------------------------------------------------------------


    // Класс для тестирования FixedMemBlockPool в режиме симулирования реального
    class MemoryPoolUsingSimulator
    {
        typedef vector<MemoryBlock> BlocksVector;

        // Memory pool
        scoped_ptr<IFixedMemBlockPool> m_pMemoryPool;
        vector<byte>        m_buffer;
        int                 m_blockSize;
        int                 m_blocksNumber;
        bool                m_debug;
        bool                m_allocThrowsException;
        bool                m_userProvidedMemory;
        BlocksVector        m_allocatedBlocks;


        Random m_random;


        // Simulation statistics
        dword m_blocksAllocated;
        dword m_allocFailedCount;
        dword m_blocksFreed;
        dword m_minBlockSize;
        dword m_maxBlockSize;
        dword m_totalBlocksSize;

        dword m_blocksFilled;
        dword m_minFillLength;
        dword m_maxFillLength;
        dword m_totalFillLength;


        //---------------------------------------------------------------------


        void InitializeStatistics()
        {
            m_blocksAllocated  = 0;
            m_allocFailedCount = 0;
            m_blocksFreed      = 0;
            m_minBlockSize     = std::numeric_limits<dword>::max();
            m_maxBlockSize     = 0;
            m_totalBlocksSize  = 0;

            m_blocksFilled     = 0;
            m_minFillLength    = std::numeric_limits<dword>::max();
            m_maxFillLength    = 0;
            m_totalFillLength  = 0;
        }


        //---------------------------------------------------------------------


        void ValidateBlockData(const MemoryBlock &block)
        {
            ESS_ASSERT(block.getBlockAddress() != 0);

            /*
            byte xor = MemoryBlock::CalculateXor(block.getBlockAddress(), block.getBlockSize());
            ESS_ASSERT(block.getXor() == xor);
            */
            byte xorByte = MemoryBlock::CalculateXor(block.getBlockAddress(), block.getBlockSize());
            ESS_ASSERT(block.getXor() == xorByte);

        }


        //---------------------------------------------------------------------


        void TryAllocateBlock()
        {
            void *pBlock;
            int blockSize = m_random.Next(m_blockSize) + 1;

            // Allocate block and add it to the vector of allocated blocks
            try
            {
                pBlock = m_pMemoryPool->alloc(blockSize);

                // Validate returned address if we are using exceptions (assert if
                // address == 0 but the exception is not raised)
                ESS_ASSERT((m_allocThrowsException != true) || (pBlock != 0));

                // No more available blocks
                if (pBlock == 0)
                {
                    ++m_allocFailedCount;
                    return;
                }
            }
            catch (const NoBlocksAvailable &e)
            {
                // No more available blocks
                ++m_allocFailedCount;
                return;
            }

            // Validate returned address if we are using user-provided memory
            ESS_ASSERT((m_userProvidedMemory != true) ||
                ((pBlock >= &m_buffer[0]) && (pBlock <= &m_buffer[0] + m_buffer.size())));

            // Fill allocated block with random data
            byte *buffer = IntPtr(pBlock).ToPtr<byte>();
            for (int i = 0; i < blockSize; ++i) buffer[i] = m_random.NextByte();

            // Store block information in the vector
            m_allocatedBlocks.push_back(MemoryBlock(pBlock, blockSize));

            // New block is allocated - update statistics
            ++m_blocksAllocated;
            m_minBlockSize    =  (blockSize < m_minBlockSize ? blockSize : m_minBlockSize);
            m_maxBlockSize    =  (blockSize > m_maxBlockSize ? blockSize : m_maxBlockSize);
            m_totalBlocksSize += m_blockSize;
        }


        //---------------------------------------------------------------------


        void FreeBlock(BlocksVector::iterator blockIterator)
        {
            MemoryBlock &block = *blockIterator;

            ESS_ASSERT(block.getBlockAddress() != 0);

            // Look for block's data corruption before freeing
            ValidateBlockData(block);

            // Free block
            m_pMemoryPool->free(block.getBlockAddress());

            // Remove from the vector
            m_allocatedBlocks.erase(blockIterator);

            // Update statistics
            ++m_blocksFreed;
        }


        //---------------------------------------------------------------------


        void FreeRandomBlock()
        {
            ESS_ASSERT(m_allocatedBlocks.empty() != true);

            int blockIndex = m_random.Next(m_allocatedBlocks.size());

            FreeBlock(m_allocatedBlocks.begin() + blockIndex);
        }


        //---------------------------------------------------------------------


        void FreeAllBlocks()
        {
            while (!m_allocatedBlocks.empty())
            {
                FreeBlock(m_allocatedBlocks.begin());
            }
        }


        //---------------------------------------------------------------------


        void ResizeBuffer(int fullBlockSize)
        {
            m_buffer.resize(fullBlockSize * m_blocksNumber);
        }


    public:
        MemoryPoolUsingSimulator(int blockSize, int blocksNumber, int extraBlocksNumber, bool debug, bool allocThrowsException) :
            m_blockSize(blockSize),
            m_blocksNumber(blocksNumber),
            m_debug(debug),
            m_allocThrowsException(allocThrowsException),
            m_userProvidedMemory(false)
        {
            ESS_ASSERT(m_blockSize         >= 0);
            ESS_ASSERT(m_blocksNumber      >  0);
            ESS_ASSERT(extraBlocksNumber   >= 0);

            if (m_debug)
            {
                typedef FixedMemBlockPool<DefaultThreadSafeStrategy, true> PoolType;
                m_pMemoryPool.reset(new PoolType(m_blockSize, m_blocksNumber, extraBlocksNumber, m_allocThrowsException));
            }
            else
            {
                typedef FixedMemBlockPool<DefaultThreadSafeStrategy, false> PoolType;
                m_pMemoryPool.reset(new PoolType(m_blockSize, m_blocksNumber, extraBlocksNumber, m_allocThrowsException));
            }

            m_blocksNumber += extraBlocksNumber;
        }


        //---------------------------------------------------------------------


        MemoryPoolUsingSimulator(int blockSize, int blocksNumber, bool debug, bool allocThrowsException) :
            m_blockSize(blockSize),
            m_blocksNumber(blocksNumber),
            m_debug(debug),
            m_allocThrowsException(allocThrowsException),
            m_userProvidedMemory(true)
        {
            ESS_ASSERT(m_blockSize    >= 0);
            ESS_ASSERT(m_blocksNumber >  0);


            if (m_debug)
            {
                typedef FixedMemBlockPool<DefaultThreadSafeStrategy, true> PoolType;

                ResizeBuffer(PoolType::GetFullBlockSize(m_blockSize));

                m_pMemoryPool.reset(new PoolType(m_blockSize, &m_buffer[0], m_buffer.size(), m_allocThrowsException));
            }
            else
            {
                typedef FixedMemBlockPool<DefaultThreadSafeStrategy, false> PoolType;

                ResizeBuffer(PoolType::GetFullBlockSize(m_blockSize));

                m_pMemoryPool.reset(new PoolType(m_blockSize, &m_buffer[0], m_buffer.size(), m_allocThrowsException));
            }
      }


      //-----------------------------------------------------------------------


        void StartSimulation(dword iterationsNumber)
        {
            InitializeStatistics();

            // Simulate allocation, freeing, and filling of blocks
            while (iterationsNumber--)
            {
                // Allocate new block or free random block
                if (m_random.Next(2) != 0)
                {
                    TryAllocateBlock();
                }
                else
                {
                    // Free random block if one or more blocks available
                    if (m_allocatedBlocks.empty() != true)
                        FreeRandomBlock();
                }
            }

            // Free remaining blocks
            FreeAllBlocks();
        }


        //---------------------------------------------------------------------


        void PrintStatisticsTo(string &statistics) const
        {
            ostringstream ss;

            // Show statistics
            ss
                << "Simulation statistics:" << endl
                << endl
                << "  Total allocated blocks:       " << m_blocksAllocated  << endl
                << "  Total allocations failed:     " << m_allocFailedCount << endl
                << "  Total freed blocks:           " << m_blocksFreed      << endl
                << endl
                << "  Minimum allocated block size: " << m_minBlockSize     << endl
                << "  Maximum allocated block size: " << m_maxBlockSize     << endl
                << "  Size of all allocated blocks: " << m_totalBlocksSize  << endl;

            statistics = ss.str();
        }
    };

}


namespace
{

    // Tests FixedMemBlockPool constructor
    template<bool debugMode>
    bool TestBadArgsConstructorHeap(int blockSize, int blocksNumber, int extraBlocksNumber)
    {
        ExceptionHookRethrow<> hook;

        try
        {
            FixedMemBlockPool<DefaultThreadSafeStrategy, debugMode> memoryPool(blockSize, blocksNumber, extraBlocksNumber);
            return false;
        }
        catch (const HookRethrow &e) {}

        return true;
    }


    //-------------------------------------------------------------------------


    // Tests FixedMemBlockPool constructor with user-provided memory
    template <bool debugMode>
    bool TestBadArgsConstructorUser(int blockSize, int blocksNumber)
    {
        ExceptionHookRethrow<> hook;
        vector<byte> buffer;


        try
        {
            int fullBlockSize = FixedMemBlockPool<DefaultThreadSafeStrategy, debugMode>::GetFullBlockSize( abs(blockSize) );
            int bufferSize = fullBlockSize * blocksNumber + 1; // "+ 1" для того, чтобы vector::operator[] не генерировал ошибку
            buffer.resize(bufferSize);

            FixedMemBlockPool<DefaultThreadSafeStrategy, debugMode> memoryPool(blockSize, &buffer[0], bufferSize);
            return false;
        }
        catch (const HookRethrow &e) {}


        return true;
    }


    //-------------------------------------------------------------------------


    // Tests FixedMemBlockPool::alloc()
    bool TestBadArgsAllocation(int blockSize, int blocksNumber, int allocationSize)
    {
        ESS_ASSERT(blockSize    >= 0);
        ESS_ASSERT(blocksNumber >  0);

        ExceptionHookRethrow<> hook; // for hooking ESS_ASSERT

        // Must work only in debug mode (arguments checking in alloc())
        FixedMemBlockPool<DefaultThreadSafeStrategy, true> memoryPool(blockSize, blocksNumber, 0);

        try
        {
            void *tempBlock = memoryPool.alloc(allocationSize, __FILE__, __LINE__);
        }
        catch (const HookRethrow &e)
        {
            // ESS_ASSERT() in the FixedMemBlockPool::alloc() function
            return true;
        }

        return false;
    }


    //-------------------------------------------------------------------------


    // Tests allocation with an excess of the limit of the blocks count
    template<bool debugMode>
    bool TestExtraBlocksAllocation(int blockSize, int blocksNumber, int extraBlocksNumber, bool allocThrowsException)
    {
        ESS_ASSERT(blockSize         >= 0);
        ESS_ASSERT(blocksNumber      >  0);
        ESS_ASSERT(extraBlocksNumber >= 0);


        FixedMemBlockPool<DefaultThreadSafeStrategy, debugMode> memoryPool(blockSize, blocksNumber, extraBlocksNumber, allocThrowsException);


        // Allocate all available blocks including extra blocks
        vector<void*> blocks;
        for (int i = 0; i < blocksNumber + extraBlocksNumber; ++i)
        {
            blocks.push_back( memoryPool.alloc(blockSize, __FILE__, __LINE__) );
        }


        bool isOk = false;
        try
        {
            // Try to allocate an extra block when all extra blocks are allocated already
            void *tempBlock = memoryPool.alloc(blockSize, __FILE__, __LINE__);

            if ((allocThrowsException == false) && (tempBlock == 0)) isOk = true;
        }
        catch (const NoBlocksAvailable &e)
        {
            // The FixedMemBlockPool::alloc() function raised the exception
            isOk = true;
        }


        // Free allocated blocks
        for (int i = 0; i < blocksNumber + extraBlocksNumber; ++i)
        {
            memoryPool.free( blocks.at(i) );
        }


        return isOk;
    }


    //-------------------------------------------------------------------------


    template<bool debugMode>
    bool TestRandomOrderFreeing(int blockSize, int blocksNumber, int extraBlocksNumber)
    {
        ESS_ASSERT(blockSize         >= 0);
        ESS_ASSERT(blocksNumber      >  0);
        ESS_ASSERT(extraBlocksNumber >= 0);


        // Must work only in all modes
        FixedMemBlockPool<DefaultThreadSafeStrategy, debugMode> memoryPool(blockSize, blocksNumber, 0);
        Random random;

        vector<void*> blocks;
        vector<void*> order;


        // Allocate all available preallocated blocks and extra blocks
        for (int i = 0; i < blocksNumber; ++i)
        {
            blocks.push_back( memoryPool.alloc(blockSize, __FILE__, __LINE__) );
        }


        // Free a blocks in random order
        while (!blocks.empty())
        {
            // Get random index
            int i = random.Next() % blocks.size();

            void *ptr = blocks.at(i);
            order.push_back(ptr);
            memoryPool.free(ptr);

            blocks.erase(blocks.begin() + i);
        }


        // Reallocate all available and extra blocks
        for (int i = 0; i < blocksNumber; ++i)
        {
            blocks.push_back( memoryPool.alloc(blockSize, __FILE__, __LINE__) );
        }


        // Compare
        vector<void*>::reverse_iterator result   = blocks.rbegin();
        vector<void*>::iterator         haveToBe = order.begin();

        bool isOk = true;
        for (; result != blocks.rend(); ++result, ++haveToBe)
        {
            if ((*result) != (*haveToBe))
            {
                isOk = false;
                break;
            }
        }

        // Free allocated blocks
        vector<void*>::iterator it = blocks.begin();
        for (; it != blocks.end(); ++it)
        {
            memoryPool.free(*it);
        }


        return isOk;
    }


    //-------------------------------------------------------------------------


    bool TestOverwriting(int defaultBlockSize, int userBlockSize, bool writeForward)
    {
        ESS_ASSERT(defaultBlockSize >  0);
        ESS_ASSERT((userBlockSize   >= 0) && (userBlockSize <= defaultBlockSize));

        // How many bytes to overwrite
        // (if we want to overwrite header, backup only 1 byte)
        const int COverwriteLength = 1 + (writeForward ? userBlockSize : 0);

        ExceptionHookRethrow<> hook;

        // Markers and headers checking available only in debug mode
        FixedMemBlockPool<DefaultThreadSafeStrategy, true> memoryPool(defaultBlockSize, 1, 0);

        // Allocate block for testing
        void *pBlock = memoryPool.alloc(userBlockSize, __FILE__, __LINE__);

        // Backup the block
        vector<byte> backup;
        backup.resize(COverwriteLength);

        byte *pOffset = IntPtr(pBlock).ToPtr<byte>() + ((writeForward) ? 0 : -1);
        memcpy(&backup[0], pOffset, COverwriteLength);


        // Overwrite marker
        memset(pOffset, 0, COverwriteLength);


        // Try to free corrupted block
        bool isOk = false;
        try
        {
            memoryPool.free(pBlock);
        }
        catch (const HookRethrow &e)
        {
            // Assertion hooked - ok
            isOk = true;
        }


        // Restore block from the backup and free it
        memcpy(pOffset, &backup[0], COverwriteLength);
        memoryPool.free(pBlock);

        return isOk;
    }


    //-------------------------------------------------------------------------


    void StartUsingSimulationHeap(int blockSize, int blocksNumber, int extraBlocksNumber, int iterationsNumber, bool debugMode, bool allocThrowsException)
    {
        ESS_ASSERT(blockSize         >= 0);
        ESS_ASSERT(blocksNumber      > 0);
        ESS_ASSERT(extraBlocksNumber >= 0);
        ESS_ASSERT(iterationsNumber  > 0);

        MemoryPoolUsingSimulator simulator(blockSize, blocksNumber, extraBlocksNumber, debugMode, allocThrowsException);
        simulator.StartSimulation(iterationsNumber);
    }


    //-------------------------------------------------------------------------


    void StartUsingSimulationUser(int blockSize, int blocksNumber, int iterationsNumber, bool debugMode, bool allocThrowsException)
    {
        ESS_ASSERT(blockSize         >= 0);
        ESS_ASSERT(blocksNumber      > 0);
        ESS_ASSERT(iterationsNumber  > 0);

        MemoryPoolUsingSimulator simulator(blockSize, blocksNumber, debugMode, allocThrowsException);
        simulator.StartSimulation(iterationsNumber);
    }

}

// ---------------------------------------------------------

namespace
{

    void FixedMemBlockPoolTest::Test()
    {
        // Constructor tests
        TUT_ASSERT( TestBadArgsConstructorHeap<false>(-CBlockSize,  CBlocksNumber, 0) );
        TUT_ASSERT( TestBadArgsConstructorHeap<false>( CBlockSize, -CBlocksNumber, 0) );
        TUT_ASSERT( TestBadArgsConstructorHeap<false>(-CBlockSize, -CBlocksNumber, 0) );
        TUT_ASSERT( TestBadArgsConstructorHeap<false>( CBlockSize,  CBlocksNumber, -CExtraBlocksNumber) );
        TUT_ASSERT( TestBadArgsConstructorHeap<true>(-CBlockSize,  CBlocksNumber, 0) );
        TUT_ASSERT( TestBadArgsConstructorHeap<true>( CBlockSize, -CBlocksNumber, 0) );
        TUT_ASSERT( TestBadArgsConstructorHeap<true>(-CBlockSize, -CBlocksNumber, 0) );
        TUT_ASSERT( TestBadArgsConstructorHeap<true>( CBlockSize,  CBlocksNumber, -CExtraBlocksNumber) );


        // Creation (constructor with user-provided memory) tests
        TUT_ASSERT( TestBadArgsConstructorUser<false>( CBlockSize, 0) );
        TUT_ASSERT( TestBadArgsConstructorUser<false>(-CBlockSize, CBlocksNumber) );
        TUT_ASSERT( TestBadArgsConstructorUser<true>( CBlockSize, 0) );
        TUT_ASSERT( TestBadArgsConstructorUser<true>(-CBlockSize, CBlocksNumber) );


        // Allocation in a memory pool tests
        TUT_ASSERT( TestBadArgsAllocation(CBlockSize, CBlocksNumber, -1) );
        TUT_ASSERT( TestBadArgsAllocation(CBlockSize, CBlocksNumber, CBlockSize + 1) );


        // Extra blocks allocation tests
        TUT_ASSERT( TestExtraBlocksAllocation<false>(CBlockSize, CBlocksNumber, 0, true) );
        TUT_ASSERT( TestExtraBlocksAllocation<false>(CBlockSize, CBlocksNumber, CExtraBlocksNumber, true) );
        TUT_ASSERT( TestExtraBlocksAllocation<true>(CBlockSize, CBlocksNumber, 0, true) );
        TUT_ASSERT( TestExtraBlocksAllocation<true>(CBlockSize, CBlocksNumber, CExtraBlocksNumber, true) );
        TUT_ASSERT( TestExtraBlocksAllocation<false>(CBlockSize, CBlocksNumber, 0, false) );
        TUT_ASSERT( TestExtraBlocksAllocation<false>(CBlockSize, CBlocksNumber, CExtraBlocksNumber, false) );
        TUT_ASSERT( TestExtraBlocksAllocation<true>(CBlockSize, CBlocksNumber, 0, false) );
        TUT_ASSERT( TestExtraBlocksAllocation<true>(CBlockSize, CBlocksNumber, CExtraBlocksNumber, false) );


        // Freeing tests
        TUT_ASSERT( TestRandomOrderFreeing<true>(CBlockSize, CBlocksNumber, CExtraBlocksNumber) );
        TUT_ASSERT( TestRandomOrderFreeing<false>(CBlockSize, CBlocksNumber, CExtraBlocksNumber) );


        // End block marker overwriting tests
        // TUT_ASSERT( TestOverwriting(CBlockSize, 0, true) );  // 0 size now not supported by class
        TUT_ASSERT( TestOverwriting(CBlockSize, CUserBlockSize, true) );
        TUT_ASSERT( TestOverwriting(CBlockSize, CBlockSize, true) );


        // Header overwriting test
        TUT_ASSERT( TestOverwriting(CBlockSize, CBlockSize, false) );
    }


    //-------------------------------------------------------------------------


    void FixedMemBlockPoolTest::UsingSimulation()
    {
        // Simulation tests of the FixedMemBlockPool (heap mode)


        // Normal using simulation (debug, w/ exceptions)
        StartUsingSimulationHeap(CBlockSize, CBlocksNumber, CExtraBlocksNumber, CIterationsNumber, true, true);

        // Using simulation without available blocks checking (debug, w/o exceptions)
        StartUsingSimulationHeap(CBlockSize, CBlocksNumber, CExtraBlocksNumber, CIterationsNumber, true, false);

        // Normal using simulation (non-debug, w/ exceptions)
        StartUsingSimulationHeap(CBlockSize, CBlocksNumber, CExtraBlocksNumber, CIterationsNumber, false, true);

        // Using simulation without available blocks checking (non-debug, w/o exceptions)
        StartUsingSimulationHeap(CBlockSize, CBlocksNumber, CExtraBlocksNumber, CIterationsNumber, false, false);


        // Simulation tests of the FixedMemBlockPool (user-provided memory mode)


        // Normal using simulation (debug, w/ exceptions)
        StartUsingSimulationUser(CBlockSize, CBlocksNumber, CIterationsNumber, true, true);

        // Normal using simulation (debug, w/o exceptions)
        StartUsingSimulationUser(CBlockSize, CBlocksNumber, CIterationsNumber, true, false);

        // Normal using simulation (non-debug, w/ exceptions)
        StartUsingSimulationUser(CBlockSize, CBlocksNumber, CIterationsNumber, false, true);

        // Normal using simulation (non-debug, w/o exceptions)
        StartUsingSimulationUser(CBlockSize, CBlocksNumber, CIterationsNumber, false, false);
    }

    // --------------------------------------------------------

    void ExceptionTest()
    {
        FixedMemBlockPool<> pool(32, 1, 0);

        void *pBlock = pool.allocFullBlock();

        bool ok = false;

        try
        {
            void *p = pool.allocFullBlock();
        }
        catch(std::exception &e)
        {
            ok = true;
        }
        catch(...)
        {
            ESS_HALT("impossible");
        }

        pool.free(pBlock);

        TUT_ASSERT(ok);
    }



}

// ---------------------------------------------------------

namespace UtilsTests
{

    void MemBlockPoolTest()
    {
        FixedMemBlockPoolTest::Test();
    }

    void MemBlockPoolTestSimulation()
    {
        FixedMemBlockPoolTest::UsingSimulation();
    }

    void MemPoolExceptionTest()
    {
        ExceptionTest();
    }

} // UtilsTests
