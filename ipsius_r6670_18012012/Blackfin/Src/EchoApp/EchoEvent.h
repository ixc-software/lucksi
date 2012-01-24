#ifndef __ECHOEVENT__
#define __ECHOEVENT__

#include "TdmMng/EventsQueue.h"

namespace EchoApp
{
        
    class EchoEvent : public TdmMng::EventsQueueItem
    { 
        const std::string m_eventName; 
        const int m_chNum; 
        const std::string m_params;

    public:

        EchoEvent(const std::string &eventName, 
                       int chNum,
                       const std::string &params) :
            m_eventName(eventName),
            m_chNum(chNum),
            m_params(params)
        {
        }

        void Send(SBProto::ISafeBiProtoForSendPack &proto) const;

    };
    
    
}  // namespace EchoApp

#endif