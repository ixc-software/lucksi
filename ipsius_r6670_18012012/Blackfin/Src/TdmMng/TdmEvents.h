#ifndef __TDMEVENTS__
#define __TDMEVENTS__

#include "eventsqueue.h"

namespace TdmMng
{
    
    class TdmAsyncEvent : public EventsQueueItem
    {
        const std::string m_source;
        const std::string m_event;
        const std::string m_params;

    public:

        TdmAsyncEvent(const std::string &src, const std::string &event, 
            const std::string &params = "") :
        m_source(src), m_event(event), m_params(params)
        {
        }

        void Send(SBProto::ISafeBiProtoForSendPack &proto) const;

        const std::string& Event() const { return m_event; }

    };

    // -------------------------------------------------------------

    struct TdmAsyncCaptureDataBody
    {
        const std::string DevName;
        const int ChNumber;

        std::vector<byte> Rx, Tx, RxFixed;

        TdmAsyncCaptureDataBody(const std::string &devName, int chNumber, int capacity) :
        DevName(devName),
            ChNumber(chNumber)
        {
            Rx.reserve(capacity);
            Tx.reserve(capacity);
            RxFixed.reserve(capacity);
        }
    };

    class TdmAsyncCaptureData : public EventsQueueItem
    {
        shared_ptr<TdmAsyncCaptureDataBody> m_body;

    public:

        TdmAsyncCaptureData(shared_ptr<TdmAsyncCaptureDataBody> body) : m_body(body)
        {
        }

        void Send(SBProto::ISafeBiProtoForSendPack &proto) const;

    };

    
}  // namespace TdmMng

#endif
