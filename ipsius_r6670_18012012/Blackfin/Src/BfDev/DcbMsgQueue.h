#ifndef DCBMSGQUEUE_H
#define DCBMSGQUEUE_H

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "Platform/PlatformTypes.h"
#include "Utils/ExecutorManager.h"

#include "InterruptLocker.h"

namespace BfDev
{      
    // todo !!! STATIC_ASSERT ADI_SSL_VDK
    
    

  
    using Platform::byte;
    using Platform::word;
    using Platform::dword;    

    class DcbMsgQueue : public boost::noncopyable
    {                   
        
        Utils::ExecutorManager<true, InterruptLockStrategy> m_packer;
        std::vector<byte> m_dcbQueueMemory;
        ADI_DCB_HANDLE m_handle;

        void PostToDcb(void* pMsg, word priority);        

    public:
        DcbMsgQueue(int deep, byte maxMsgSize);

        ~DcbMsgQueue();

        template<typename TFn, typename TArg>
        void PutDcbMsg(TFn callbackFn, const TArg& arg, word priority = 5/*default*/)
        {                        
            void* pMsg = m_packer.CreateExecutor(callbackFn, arg);                        
            PostToDcb(pMsg, priority);         
        }
        
        template<typename TOwner, typename TFn, typename TArg>
        void PutDcbMsg(TOwner owner, TFn callbackFn, const TArg& arg, word priority = 5/*default*/)
        {                        
            void* pMsg = m_packer.CreateExecutor(owner, callbackFn, arg);                        
            PostToDcb(pMsg, priority);         
        }
        
        template<typename TOwner, typename TFn>
        void PutDcbMsg(TOwner owner, TFn callbackFn, word priority = 5/*default*/)
        {                        
            void* pMsg = m_packer.CreateExecutor(owner, callbackFn);                        
            PostToDcb(pMsg, priority);         
        }
        
        ADI_DCB_HANDLE getHandle() const {return m_handle;}
    };
   
    //--------------------------------------------------------------------------
   

} // namespace BfDev

#endif
