#ifndef __VDKTHREADDCB__
#define __VDKTHREADDCB__

#include "Platform/Platform.h"
#include "BfDev/EventTime.h"
#include "iVDK/VdkThread.h"
#include "iVDK/Semaphore.h"
#include "iVDK/CriticalRegion.h"
#include "Utils/FixedFifo.h"
#include "Utils/VirtualInvoke.h"
#include "Utils/MemoryPool.h"

namespace BfDev
{
	
	class VdkThreadDcb : 
        public iVDK::detail::IThreadBody,
        boost::noncopyable
	{
        struct FifoRec
        {
            void *PBlock;
            Utils::IVirtualInvoke *PInvoke;

            FifoRec()
            {
                PBlock = 0;
                PInvoke = 0;
            }

            FifoRec(void *pBlock, Utils::IVirtualInvoke *pInvoke)
            {
                PBlock = pBlock;
                PInvoke = pInvoke;

                ESS_ASSERT(!Empty());
            }

            bool Empty()
            {
                return (PBlock == 0) || (PInvoke == 0);
            }
        };

        typedef Utils::Detail::ThreadStrategyClass<false> PoolThreadStrategy;
        static const bool CPoolDebug = true;

        iVDK::Thread m_thread;
        volatile bool m_threadBreak;

        iVDK::Semaphore m_semaphore;  // for thread wake up
        Utils::FixedFifo<FifoRec> m_fifo;
        Utils::FixedMemBlockPool<PoolThreadStrategy, CPoolDebug> m_pool;

        BfDev::EventTime m_event;

        void ThreadWakeUp()
        {
            m_semaphore.Post();
        }

        bool SyncPeekMsg(FifoRec &rec)
        {
            iVDK::CriticalRegion region;

            if (m_fifo.IsEmpty()) return false;

            rec = m_fifo.Pop();

            return true;
        }

        void SyncFreeBlock(void *p)
        {
            iVDK::CriticalRegion region;

            m_pool.free(p);
        }

        void DebugExecute(FifoRec *p)
        {
            p->PInvoke->Execute();
            p->PInvoke->~IVirtualInvoke();

            m_pool.free(p->PBlock);
        }
        
        static void AllocDcb();
        static void FreeDcb();
               
    // IThreadBody impl
    private:

        void ThreadBody()
        {
            while(true)
            {
                // peek msg
                FifoRec rec;

                while( SyncPeekMsg(rec) )
                {
                    ESS_ASSERT( !rec.Empty() );

                    // exec
                    {
                        m_event.Enter();
                        rec.PInvoke->Execute();
                        m_event.Leave();
                    }

                    // destruct and free block
                    rec.PInvoke->~IVirtualInvoke();
                    SyncFreeBlock(rec.PBlock);
                }

                // sleep
                m_semaphore.Pend();
                if (m_threadBreak) break;
            }
        }


	public:
		
		VdkThreadDcb(int queueSize, 
                     int invokeBlockSize = 64, VDK::Priority prio = VDK::kPriority1) : 
            m_thread(*this),
            m_fifo(queueSize),
            m_pool(invokeBlockSize, queueSize, 0, false)
		{
			AllocDcb();
			
            m_threadBreak = false;

			m_thread.Start(prio);
		}

        ~VdkThreadDcb()
        {
            enum { CMaxWaitTime = 50, };

            m_threadBreak = true;

            int t = Platform::GetSystemTickCount();

            while(true)
            {
                ThreadWakeUp();
                Platform::ThreadSleep(1);
                
                // manual destroy, 'couse CpuUsage block Idle thread which must clean destroyed thread
                VDK::FreeDestroyedThreads();

                // thread exit
                if (!m_thread.IsRunning()) break;

                // timeout
                if (Platform::GetSystemTickCount() - t > CMaxWaitTime) 
                {
                    ESS_HALT("VdkThreadDcb break timeout");
                }
            }

            // m_thread.Wait();

            // Is it allow to kill non processed messages - ?!
            ESS_ASSERT(m_fifo.Size() == 0);
            
            FreeDcb();
        }

        // called from IRQ context!
        // use for -- void TBase::fn(T param)
        template<class TFn, class TBase, class TParam>
        void PutMsg(TFn fn, TBase &base, TParam &param)
        {
            typedef Utils::VIMethodWithParam<TBase, TParam> VI;

            ESS_ASSERT(!m_threadBreak);

            ESS_ASSERT(m_pool.getBlockSize() >= sizeof(VI));

            bool insideIrq = Platform::InsideIrqContext();

            // use CriticalRegion only outside IRQ
            // non IRQ use implemented just for test
            if (!insideIrq) iVDK::CriticalRegion::Enter();

            // protected region
            {
                void *pBlock = m_pool.alloc(sizeof(VI));

                if (pBlock == 0)
                {
                    m_event.Check();
                    ESS_HALT("VdkThreadDcb blocks out!");
                }
                

                VI *pInvoke = new(pBlock) VI(fn, base, param);

                FifoRec rec(pBlock, pInvoke);

                m_fifo.Push(rec);

                // debug
                // DebugExecute(&rec);
            }

            if (!insideIrq) iVDK::CriticalRegion::Leave();

            // thread wakeup
            ThreadWakeUp();
        }
       	
	};

    void VdkThreadDcbTest();
	
	
} // namespace BfDev

#endif
