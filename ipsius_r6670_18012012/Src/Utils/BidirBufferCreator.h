#ifndef BUFFERCREATOR_H
#define BUFFERCREATOR_H

#include "ProjConfigLocal.h"

#include "Utils/MemoryPool.h"
#include "Utils/BidirBuffer.h"
#include "Utils/IBidirBuffCreator.h"

namespace Utils
{

    template<
        bool DebugMode  = ProjConfig::CfgMemoryPool::CEnableDebug,
        bool ThreadSafe = ProjConfig::CfgMemoryPool::CEnableThreadSafe
             >
    class BidirBufferCreator : public Utils::IBidirBuffCreator
    {
        typedef Utils::Detail::ThreadStrategyClass<ThreadSafe> PoolThreadStrategy;
        typedef Utils::FixedMemBlockPool<PoolThreadStrategy, DebugMode> MemoryPool;

        MemoryPool m_poolObj;
        MemoryPool m_poolBuff;
        int m_defaultOffset;

    public:

        BidirBufferCreator(int blockSize, int count, int defaultOffset = -1, int maxAllocFromHeap = 0) :
          m_poolObj(Utils::BidirBuffer::AllocSizeof(), count, maxAllocFromHeap, true),
          m_poolBuff(blockSize,                        count, maxAllocFromHeap, true),
          m_defaultOffset(defaultOffset)
        {
            // nothing
        }

        static void BidirCreatorTest()
        {
            int maxBlocks = 2;

            BidirBufferCreator<> creator(32, maxBlocks, 0);

            std::vector<BidirBuffer*> m_list;

            for(int i = 0; i < maxBlocks; ++i) m_list.push_back( creator.CreateBidirBuff() );

            bool wasException = false;

            try
            {
                creator.CreateBidirBuff();  // throw
            }
            catch(/*const*/ NoBlocksAvailable &e)
            {
                wasException = true;
            }

            TUT_ASSERT(wasException);

            // free
            for(int i = 0; i < maxBlocks; ++i) delete m_list.at(i);
        }

    // IBidirBuffCreator impl
    public:

        Utils::BidirBuffer* CreateBidirBuff(int offset)
        {
            return new(m_poolObj) Utils::BidirBuffer(m_poolBuff, offset);
        }

        Utils::BidirBuffer* CreateBidirBuff()
        {
            ESS_ASSERT(m_defaultOffset >= 0);

            return CreateBidirBuff(m_defaultOffset);
        }

    };

} // namespace TdmMng

#endif

