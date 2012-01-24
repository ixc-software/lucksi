#include "stdafx.h"
#include "Utils/MemoryPoolBody.h"

#include "DcbMsgQueue.h"


namespace 
{
    using Platform::byte;
    
    const byte CDcbQueueCapacity = 7; // количество очередей поддерживемых DcbManager
    
    // adi_dcb callback function
    void GeneralCallback (void* pExecutor, u32, void*)
    {        
        Utils::ExecutorManager<true, BfDev::InterruptLockStrategy>::Execute(pExecutor);        
    }
    
    // C++ wraper of adi_dcb API
    class DcbManager
    {
        std::vector<byte> m_dcbMemKeeper; // выделенние памяти для adi_dcb
        int m_currQueueCount;
        ADI_DCB_CALLBACK_FN m_generalCallbackFn;
        
        //static const Platform::dword CQueueServerMemorySize = (ADI_DCB_QUEUE_SIZE) * CQueueServersNumber;
    private:
    
        DcbManager(byte maxQueueCount)
            : m_dcbMemKeeper( (ADI_DCB_QUEUE_SIZE) * maxQueueCount, 0 ),
            m_currQueueCount(0),
            m_generalCallbackFn(GeneralCallback)
        {            
            // Initialize deferred callback manager
            u32 queueCount = 0;
            
            ADI_DCB_RESULT errorCode = adi_dcb_Init
            (
                &m_dcbMemKeeper[0],
                m_dcbMemKeeper.size(),
                &queueCount,
                0 // todo: разобраться с CriticalRegion
            );
            
            ESS_ASSERT( errorCode == ADI_DCB_RESULT_SUCCESS );
            ESS_ASSERT( queueCount == m_dcbMemKeeper.size() / (ADI_DCB_QUEUE_SIZE));
        }
        
        ~DcbManager()
        {
            ESS_ASSERT (m_currQueueCount == 0);
        }
        
        /*byte getMaxQueueCount() const
        {
            return m_dcbMemKeeper.size() / (ADI_DCB_QUEUE_SIZE);
        }*/
        
    public:    
    
        static DcbManager& Instance()
        {
            static DcbManager mng(CDcbQueueCapacity);
            return mng;
        }
        
        ADI_DCB_HANDLE RegQueue( std::vector<byte>& queueMemory )
        {
            ++m_currQueueCount;            
            ESS_ASSERT( m_dcbMemKeeper.size() / (ADI_DCB_QUEUE_SIZE) >=  m_currQueueCount);
            
            u32 queueEntries = 0;
            ADI_DCB_HANDLE queueHandle = 0;
            ADI_DCB_RESULT errorCode = adi_dcb_Open
            (
                0, // ignored in VDK
                &queueMemory[0],
                queueMemory.size(),
                &queueEntries,
                &queueHandle
            );

            ESS_ASSERT(errorCode        == ADI_DCB_RESULT_SUCCESS);
            ESS_ASSERT(queueEntries     == queueMemory.size() / ADI_DCB_ENTRY_SIZE);
            ESS_ASSERT(queueHandle != 0);
                                               
            return queueHandle;
        }
        
        void UnregQueue(ADI_DCB_HANDLE handle)
        {
            --m_currQueueCount; 
            
            ESS_ASSERT(adi_dcb_Close(handle) == ADI_DCB_RESULT_SUCCESS);           
        }
        
        void Post(ADI_DCB_HANDLE hQueue, Platform::word priority, void* pArg)
        {
            ADI_DCB_RESULT result = adi_dcb_Post
                (
                    hQueue,
                    priority,
                    m_generalCallbackFn,
                    pArg,
                    0,
                    0
                );
                
            ESS_ASSERT(result ==  ADI_DCB_RESULT_SUCCESS);
        }
        
    };
    
} // namespace 

namespace BfDev
{           
    /*
    byte SizeAlign(byte size)    
    {
        int notAlign = size % CMemoryAlign;
        if (notAlign)        
            size += CMemoryAlign - notAlign;
        return size;
    }
    */

    DcbMsgQueue::~DcbMsgQueue()
    {
        // дерегистрация в синглтоне dcb
        DcbManager::Instance().UnregQueue(m_handle);
    }

    DcbMsgQueue::DcbMsgQueue( int deep, byte maxMsgSize) 
        : m_packer(maxMsgSize, deep),
        m_dcbQueueMemory( (ADI_DCB_ENTRY_SIZE) * deep, 0 ) // allocate for using in adi_dcb
    {               
        // регистрация в синглтоне dcb, получение handle
        m_handle = DcbManager::Instance().RegQueue(m_dcbQueueMemory);
    }

    void DcbMsgQueue::PostToDcb( void* pMsg, word priority )
    {
        DcbManager::Instance().Post(m_handle, priority, pMsg);
    }

    
} // namespace BfDev

