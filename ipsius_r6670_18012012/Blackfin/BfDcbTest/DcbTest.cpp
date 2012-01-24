#include "DcbTest.h"
#include "BfDev/DcbMsgQueue.h"
#include "Utils/ErrorsSubSystem.h"


#include "BfDev/AdiGPTimer.h"

namespace BfDcbTest
{        
    
    //using Platform::dword;                       
  
                
    
    
    class DcbQueueTest
    {
        typedef DcbQueueTest TMy;
        
        enum TestParam
        {
            CQueueDeep = 3,
            CDuration = 0x08000000,
            CInterruptPeriod = 0x00800000,
            CqueueArg = 8792,
            CInterruptArg = 8768,
        };    
        
        BfDev::AdiGPTimer m_interruptTimer;
        BfDev::AdiGPTimer m_stopTimer;
        
        int m_sendToQueueCount;
        int m_rcvFromQueueCount;
        BfDev::DcbMsgQueue m_dcbQueue;
        bool m_testIsActive; // AtomicBool
        
        void OnInterrupt(const int& arg)
        {
            TUT_ASSERT(arg == CInterruptArg);
            m_sendToQueueCount++;
            int queueArg = CqueueArg;
            m_dcbQueue.PutDcbMsg(this, &TMy::OnDcbMsg, queueArg);        
        }
        
        void OnDcbMsg(const int& num)
        {   
            m_rcvFromQueueCount++;       
            TUT_ASSERT(num == CqueueArg);        
        }
        
        void OnDcbStop()
        {           
            m_testIsActive = false;
        }
    
        void OnStop()
        {
            if(!m_testIsActive)
                return;
        
            {
                BfDev::InterruptLocker lock;
                TUT_ASSERT(m_sendToQueueCount != 0);
            }                        
        
            TUT_ASSERT(m_rcvFromQueueCount != 0);
            m_dcbQueue.PutDcbMsg(this, &TMy::OnDcbStop);
        }
        
     public:   
     
        DcbQueueTest()
            : m_interruptTimer(ADI_TMR_GP_TIMER_0), //m_interruptTimer(BfDev::AdiGPTimer::Instance<0>()),
            m_stopTimer(ADI_TMR_GP_TIMER_1), //m_stopTimer(BfDev::AdiGPTimer::Instance<1>()),                       
            m_sendToQueueCount(0),
            m_rcvFromQueueCount(0),
            m_dcbQueue(3, 36),
            m_testIsActive(false)            
        {            
        }
        
        ~DcbQueueTest()
        {
            m_interruptTimer.Stop();
            m_stopTimer.Stop();
        }
        
        void Run()
        {
            int interruptArg = CInterruptArg;
            m_interruptTimer.Start(this, &TMy::OnInterrupt, interruptArg, CInterruptPeriod);
            m_stopTimer.Start(this, &TMy::OnStop, CDuration);
            m_testIsActive = true;
            
            while(m_testIsActive);            
        }
        
    };
    
   
}


extern "C"
{
    void DcbTest()
    {        
        {        
            BfDcbTest::DcbQueueTest test;
            test.Run(); 
        }
        
        ESS_HALT("Test Complete"); //break
    }
}
