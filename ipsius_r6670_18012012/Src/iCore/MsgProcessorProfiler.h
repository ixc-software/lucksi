#ifndef __MSGPROCESSORPROFILER__
#define __MSGPROCESSORPROFILER__

#include "Platform/Platform.h"

#include "Utils/SafeRef.h"
#include "Utils/FixedFifo.h"

#include "MsgProcessor.h"

namespace iCore
{
    
    class MsgProcessorProfiler : public IMsgProcessorProfiler
    {
        struct MsgPair
        {
            MsgBase *Msg;
            std::string MsgInfo;
            Platform::ddword StartTimestamp;
        };

        Utils::FixedFifo<MsgPair> m_fifo;
        MsgPair m_curr;
        Platform::dword m_msgProcessed;

    // IMsgProcessorProfiler impl
    private:

        void MsgBegin(const shared_ptr<MsgBase> &msg)
        {
            m_curr.Msg = msg.get();
            m_curr.MsgInfo = typeid(*msg).name();
            m_curr.StartTimestamp = Platform::GetCurrentThreadTimesMcs();
        }

        void MsgEnd(const shared_ptr<MsgBase> &msg)
        {
            if (msg.get() != m_curr.Msg) return;

            // fix msg processing time
            m_curr.StartTimestamp = 
                Platform::GetCurrentThreadTimesMcs() - m_curr.StartTimestamp;
            
            if (m_fifo.IsFull()) m_fifo.Pop();
            m_fifo.Push(m_curr);
            ++m_msgProcessed;
        }

        void GetDebugInfo(std::string &info) const
        {
            Utils::FixedFifo<MsgPair> q = m_fifo; // it's expensive

            std::ostringstream oss;

            while(!q.IsEmpty())
            {
                MsgPair pair = q.Pop();
                oss << pair.StartTimestamp << " mcs " << pair.MsgInfo << "\n";
            }

            info = oss.str();
        }

    public:

        MsgProcessorProfiler(int buffSize) : m_fifo(buffSize), m_msgProcessed(0)
        {
        }
                
    };
        
}  // namespace iCore

#endif
