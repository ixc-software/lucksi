#ifndef __EVENTSQUEUE__
#define __EVENTSQUEUE__

#include "Platform/Platform.h"

#include "Utils/ManagedList.h"

namespace SBProto
{
    class ISafeBiProtoForSendPack;
}

namespace TdmMng
{
    using Platform::byte;
    using boost::shared_ptr;

    // -----------------------------------------------------------

    enum AozLineEvent
    {
        aleDialBegin,   // начало набора первой цифры (событие для остановки генератора в линию)

        // эти события используются для автоматического управления DTMF приемником
        aleBoardOn,     // плата доступна
        aleBoardOff,    // плата недоступна
        aleLineBusy,    // абонент снял трубку
        aleLineFree,    // абонент положил трубку
    };

    class IIEventsQueueItemDispatch : public Utils::IBasicInterface
    {
    public:
        virtual void OnAozLineEvent(const std::string &devName, int chNum, AozLineEvent e) = 0;
    };

    // -----------------------------------------------------------

    // base class for all types, stored in EventsQueue
    class EventsQueueItem 
    {

        virtual std::string ToStringImpl() const
        {
            return typeid(this).name();
        }

        virtual void DispatchImpl(IIEventsQueueItemDispatch &callback) const
        {
            // nothing, for overriding 
        }

    protected:

        EventsQueueItem()
        {
        }

    public:

        std::string ToString() const
        {
            return ToStringImpl();
        }

        virtual void Dispatch(IIEventsQueueItemDispatch &callback) const
        {
            DispatchImpl(callback);
        }

    };
    
    // -------------------------------------------------------------

    class IAsyncQueueSend : public Utils::IBasicInterface
    {
    public:

        virtual void  Send(SBProto::ISafeBiProtoForSendPack &proto) = 0;
        virtual const EventsQueueItem& Data() = 0;
        virtual void  Dispatch(IIEventsQueueItemDispatch &callback) = 0;
    };

    // send polymorph wrapper, IAsyncQueueSend impl for all EventsQueueItem types
    template<class T>
    class TypeSend : public IAsyncQueueSend
    {
        const T m_data;

    // IAsyncQueueSend impl
    private:

        void Send(SBProto::ISafeBiProtoForSendPack &proto)
        {
            m_data.Send(proto);
        }

        const EventsQueueItem& Data() 
        {
            return m_data;
        }

        void  Dispatch(IIEventsQueueItemDispatch &callback)
        {
            m_data.Dispatch(callback);
        }

    public:

        TypeSend(const T &data) : m_data(data)
        {
        }

    };

    // -------------------------------------------------------------

    class EventsQueue : boost::noncopyable
    {
        Utils::ManagedList<IAsyncQueueSend> m_queue;

        const std::string m_name;
        int m_limit;
        bool m_overflow;

    public:

        EventsQueue( const std::string &name, int limit = 128) : 
          m_name(name), m_limit(limit), m_overflow(false)
        {
        }

        void Clear()
        {
            m_queue.Clear();
        }

        template<class T>
        void Push(const T &data)
        {
            if (m_queue.Size() > m_limit)
            {
                // drop half items
                m_queue.Delete(0, m_limit / 2);

                m_overflow = true;
            }

            m_queue.Add( new TypeSend<T>(data) );
        }

        void SendAll(SBProto::ISafeBiProtoForSendPack &proto, IIEventsQueueItemDispatch *pDispatch = 0);
    };
    
}  // namespace TdmMng

#endif
