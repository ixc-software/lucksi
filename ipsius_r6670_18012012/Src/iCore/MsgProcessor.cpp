#include "stdafx.h"
#include "MsgProcessor.h"


// -----------------------------------------------------

namespace iCore
{

    void MsgProcessor::ExecuteMsgWithHook( MsgBase *pT )
    {
        try
        {
            pT->Process();
        }
        catch(/*const*/ std::exception &e)
        {
            std::string msgType = typeid(*pT).name();
            std::string eType   = typeid(e).name();
            std::string eInfo   = e.what();

            // process hooks
            bool ignore = false;
            for(int i = 0; i < m_hooks.size(); ++i)
            {
                try
                {
                    bool suppressIt = false; 
                    m_hooks.at(i)->OnMsgException(e, pT, suppressIt);
                    if (suppressIt) ignore = true;
                }
                catch(/* const */ std::exception &e)
                {
                    ESS_UNEXPECTED_EXCEPTION(e);
                }
            }

            // ignore exception
            if (ignore) return;

            // ... or re-throw 
            throw;
        }

    }

    void MsgProcessor::MsgProcessEnd()
    {
        if (!m_profiler.IsEmpty()) m_profiler->MsgEnd(m_currentMsg);
        m_currentMsg.reset();            
    }

    void MsgProcessor::DoProcessMsg(MsgBase *pT)
    {
        ESS_ASSERT(pT != 0);

        if(m_currentMsg.get() != 0)
        {
            ESS_HALT("");
        }

        m_currentMsg.reset(pT);  // take ownership

        if (!m_profiler.IsEmpty()) m_profiler->MsgBegin(m_currentMsg);

        try
        {
            ExecuteMsgWithHook( m_currentMsg.get() );
        }
        catch(...)
        {
            // exit with exception
            MsgProcessEnd();

            throw;
        }

        // normal exit
        MsgProcessEnd();
    }

}  // namespace iCore


