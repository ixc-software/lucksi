#pragma once

#include "MsgThread.h"
#include "MsgBase.h"

namespace iCore
{
    
    // RAII + functor map for IMsgProcessorErrorHook
    class MsgErrHookFn : 
        public Utils::SafeRefServer, 
        public IMsgProcessorErrorHook,
        boost::noncopyable
    {
        typedef boost::function<bool (const std::exception&, MsgBase*)> HookFn;

        iCore::MsgThread &m_t;
        HookFn m_fn;

    // IMsgProcessorErrorHook impl
    private:

        void OnMsgException(const std::exception &e, MsgBase *pMsg, 
            /* out */ bool &suppressIt)
        {
            suppressIt = m_fn(e, pMsg);
        }

    public:

        MsgErrHookFn(iCore::MsgThread &t, const HookFn &fn) : m_t(t), m_fn(fn)
        {
            ESS_ASSERT(fn);

            m_t.AddErrorHook(this);
        }

        ~MsgErrHookFn()
        {
            m_t.RemoveErrorHook(this);
        }

    };
    
    
}  // namespace iCore