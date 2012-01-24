#ifndef __MSDATETIME__
#define __MSDATETIME__

#include "Platform/Platform.h"
#include "Utils/UtilsDateTime.h"

/*
    Реализация захвата даты и времени на основе "точек синхронизации" и миллисекундного таймера

*/

namespace Utils
{
    using Platform::ddword;
    using Platform::dword;
    using Platform::word;
    using Platform::byte;

    // ----------------------------------------------------

    namespace Detail
    {

        // 64-х разрядный счетчик миллисекундных тиков в системе
        struct LongCounter
        {
            dword lo;
            dword hi;
			
			LongCounter() : lo(0), hi(0) { }

            void SetToDDword(ddword &val) const
            {
                val = hi;
                val <<= 32;
                val += lo;
            }
        };

        // точка соотвествия -- LongCounter <-> DateTime
        struct SyncPoint
        {
            DateTime    datetime;
            LongCounter counter;
        };

        // LongCounter + SyncPoint, относительно которой он был создан
        struct Timestamp
        {
            LongCounter counter;
            SyncPoint   *pSyncPoint;
			
			Timestamp() : pSyncPoint(0) {}
        };

    };

    // ----------------------------------------------------

    class MsDateTime : 
        public ComparableT<MsDateTime>
    {
        enum State
        {
            stNull,      
            stCaptured,  // m_timestamp correct
            stResolved,  // m_datetime  correct
        };

        mutable State m_state;
        Detail::Timestamp m_timestamp;
        mutable DateTime m_datetime;

        void ResolveIsNeeded() const
        {
            ESS_ASSERT(!IsNull());

            if (m_state == stResolved) return;

            m_state = stResolved;
            ResolveTimestamp(m_timestamp, m_datetime);
        }

        static void GetTimestamp(Detail::Timestamp &ts);
        static void ResolveTimestamp(const Detail::Timestamp &ts, DateTime &dt);

    public:

        MsDateTime()
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
            GetTimestamp(m_timestamp);
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

        int Compare(const MsDateTime &other) const;

        static void SetSyncPoint(const DateTime &datetime); 

        // TODO добавить set функции - ??
        // полученный результат можно сразу хранить в m_datetime

    };
    
}  // namespace Utils


#endif

