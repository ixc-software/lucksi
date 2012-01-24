#include "stdafx.h"

#include "eventsqueue.h"

#include "iCmp/ChMngProto.h"

namespace TdmMng
{
           
    void EventsQueue::SendAll( SBProto::ISafeBiProtoForSendPack &proto, IIEventsQueueItemDispatch *pDispatch )
    {
        if (m_overflow)
        {
            m_overflow = false;
            TdmAsyncEvent e(m_name, iCmp::BfTdmEvent::CQueueOverflow());
            e.Send(proto);
        }

        for(int i = 0; i < m_queue.Size(); ++i)
        {
            if (pDispatch != 0) m_queue[i]->Dispatch(*pDispatch);
            m_queue[i]->Send(proto);
        }

        m_queue.Clear();
    }

}  // namespace TdmMng
