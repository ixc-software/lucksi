#ifndef __QTDATETIME__
#define  __QTDATETIME__

#include "Utils/ErrorsSubsystem.h"
#include "Utils/UtilsDateTime.h"

namespace Utils
{
	
    class QtDateTime :
        public ComparableT<QtDateTime>
    {
        enum State
        {
            stNull,      
            stCaptured,  // m_timestamp correct
            stResolved,  // m_datetime  correct
        };

        mutable State m_state;
        QDateTime m_timestamp;
        mutable DateTime m_datetime;

        void ResolveIsNeeded() const
        {
            ESS_ASSERT(!IsNull());

            if (m_state == stResolved) return;

            m_state = stResolved;

            m_datetime.date().Set(m_timestamp.date().year(), m_timestamp.date().month(), 
                m_timestamp.date().day());

            m_datetime.time().Set(m_timestamp.time().hour(), m_timestamp.time().minute(), 
                m_timestamp.time().second(), m_timestamp.time().msec());
        }

    public:

        QtDateTime()
        {
            Clear(); 
        }

        void Clear()
        {
            m_state = stNull;
        }

        bool IsNull() const
        {
            return (m_state == stNull);
        }

        void Capture()
        {
            m_timestamp = QDateTime::currentDateTime();
            m_state = stCaptured;
        }

        const Date& date() const
        {
            ResolveIsNeeded();

            return m_datetime.date();
        }

        const Time& time() const
        {
            ResolveIsNeeded();

            return m_datetime.time();
        }

        const DateTime& DateAndTime() const
        {
            ResolveIsNeeded();

            return m_datetime;
        }

        int Compare(const QtDateTime &other) const
        {
            // return DateAndTime().Compare( other.DateAndTime() );

            if (IsNull()) return (other.IsNull())? 0 : -1;
            if (other.IsNull()) return (IsNull())? 0 : 1;

            return DateAndTime().Compare( other.DateAndTime() );
        }

    };

	
	
}  // namespace Utils

#endif

