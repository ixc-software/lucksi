#ifndef __MSGPROCESSOR__

#define __MSGPROCESSOR__

#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"

#include "stdafx.h"
#include "MsgBase.h"

namespace iCore
{
    using boost::shared_ptr;

    // ---------------------------------------------------

    class IMsgProcessorProfiler : public virtual Utils::SafeRefServer
    {
    public:        
        virtual void MsgBegin(const shared_ptr<MsgBase> &msg) = 0; 
        virtual void MsgEnd(const shared_ptr<MsgBase> &msg) = 0;

        virtual void GetDebugInfo(std::string &info) const = 0;
    };

    // ---------------------------------------------------

    class IMsgProcessorErrorHook : public Utils::IBasicInterface
    {
    public:
        virtual void OnMsgException(const std::exception &e, MsgBase *pMsg, 
            /* out */ bool &suppressIt) = 0;
    };

    // ---------------------------------------------------
	
    // ќбъект, позвол€ющий "исполн€ть" сообщени€ с перехватом исключений и профилированием
    class MsgProcessor
    {
        typedef std::vector< Utils::SafeRef<IMsgProcessorErrorHook> > HookList;

        HookList m_hooks;  
        shared_ptr<MsgBase> m_currentMsg;
        Utils::SafeRef<IMsgProcessorProfiler> m_profiler;

        void ExecuteMsgWithHook(MsgBase *pT);
        void MsgProcessEnd();

        template<class T>
        void VerifyField(const Utils::SafeRef<T> &param, 
                         const Utils::SafeRef<T> &field)
        {
            if (!field.IsEmpty())
            {
                ESS_ASSERT( param.IsEmpty() );
            }
        }
       
    public:

        MsgProcessor() 
        {
        }

        void DoProcessMsg(MsgBase *pT);

        MsgBase* Current() { return m_currentMsg.get(); }

        void SetProfiler(Utils::SafeRef<IMsgProcessorProfiler> profiler)
        {
            VerifyField(profiler, m_profiler);
            m_profiler = profiler;
        }

        /*
        void SetErrorHook(Utils::SafeRef<IMsgProcessorErrorHook> hook)
        {
            VerifyField(hook, m_hook);
            m_hook = hook;
        } */

        void AddErrorHook(Utils::SafeRef<IMsgProcessorErrorHook> hook)
        {
            ESS_ASSERT( !hook.IsEmpty() );
            m_hooks.push_back(hook);
        }

        void RemoveErrorHook(Utils::SafeRef<IMsgProcessorErrorHook> hook)
        {
            ESS_ASSERT( !hook.IsEmpty() );

            HookList::iterator i = std::find(m_hooks.begin(), m_hooks.end(), hook);
            ESS_ASSERT( i != m_hooks.end() );
            m_hooks.erase(i);
        }


    };  

	
} // namespace iCore


#endif

