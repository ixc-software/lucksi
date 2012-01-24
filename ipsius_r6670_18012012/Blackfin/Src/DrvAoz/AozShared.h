#ifndef __AOZSHARED__
#define __AOZSHARED__

#include "TdmMng/EventsQueue.h"

#include "Utils/IBasicInterface.h"

namespace DrvAoz
{

    struct AozEvent
    {
        int ChNumber;
        std::string Name; 
        std::string Params;

        AozEvent(int chNumber, const std::string &name, const std::string &params) :
            ChNumber(chNumber), Name(name), Params(params)
        {
        }
    };

    // ---------------------------------------

    class AbAozEvent : public TdmMng::EventsQueueItem
    {
        const std::string m_devName; 
        const AozEvent m_event;

        void DispatchImpl(TdmMng::IIEventsQueueItemDispatch &callback) const;  // override

    public:

        AbAozEvent(const std::string &devName, const AozEvent &e) : 
          m_devName(devName),
          m_event(e)
        {
        }

        void Send(SBProto::ISafeBiProtoForSendPack &proto) const;
    };

    // ---------------------------------------

    class AozTdmOutput;

    class IChannelOwner : public Utils::IBasicInterface
    {
    public:

        virtual void PushChannelEvent(const AozEvent &event) = 0;
        virtual AozTdmOutput& TdmOutput() = 0;
        virtual bool DebugTraceEnabled() = 0;

        // logging
        virtual void ChannelLog(const std::string &s) = 0;
    };
    
    
}  // namespace DrvAoz

#endif
