#ifndef __UTILSDATETIME__
#define __UTILSDATETIME__

#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ComparableT.h"

namespace Utils
{
    using Platform::ddword;
    using Platform::dword;
    using Platform::word;
    using Platform::byte;

    // -----------------------------------------------
	
    class Date : public ComparableT<Date>
    {
        bool m_isNull;

        byte m_day;
        byte m_month;
        word m_year;

        mutable int m_compareValue;  // value for quick compare

        void AssertNotNull() const
        {
            ESS_ASSERT(!m_isNull);
        }

        void ResetCompareValue()
        {
            m_compareValue = -1;
        }

        int ToInteger() const
        {
            if (m_compareValue > 0) return m_compareValue;

            {
                int val = m_day;
                val += ((int)m_month << 8);
                val += ((int)m_year << 16);
                m_compareValue = val;
            }

            return m_compareValue;
        }

    public:

        ESS_TYPEDEF(BadDate);

        Date()
        {
            Clear();
        }

        Date(word year, byte month, byte day)
        {
            if (!Set(year, month, day)) ESS_THROW(BadDate);
        }

        void Clear()
        {
            m_isNull = true;
        }

        bool Set(word year, byte month, byte day)
        {
            m_year = year;
            m_month = month;
            m_day = day;

            m_isNull = false;

            if (!IsValid()) 
            {
                m_isNull = true;
                return false;
            }

            ResetCompareValue();

            return true;
        }

        bool IsNull() const
        {
            return m_isNull;
        }

        bool IsValid() const;
        
        byte Day() const   { AssertNotNull(); return m_day; }
        byte Month() const  { AssertNotNull(); return m_month; }
        word Year() const   { AssertNotNull(); return m_year; }

        void NextDay();
        void AddDaysToDate(dword days);  // TODO -- optimise, can work really slow...

        int Compare(const Date &other) const;

        // Days since 1 AD
        dword AsDays() const;

        
        ESS_TYPEDEF(InvalidDateFormat);
        
        /*
        Format:
        d    the day as number without a leading zero (1 to 31);
        dd   the day as number with a leading zero (01 to 31;
        M    the month as number without a leading zero (1-12)
        MM   the month as number with a leading zero (01-12)
        MMM  short month name (Jan)
        MMMM long month name (January)
        yy   the year as two digit number (00-99)
        yyyy the year as four digit number
        */
        std::string ToString(const std::string &format = "d.M.yyyy") const; // can throw

        /*
        If input string is empty or parsing is failed an invalid Date will be returned,
        For any field that is not represented in the format the following defaults are used:
        Year = 1900, Month = 1, Day = 1.

        Format:
        d    the day as number without a leading zero (1 to 31);
        dd   the day as number with a leading zero (01 to 31;
        M    the month as number without a leading zero (1-12)
        MM   the month as number with a leading zero (01-12)
        yy   the year as two digit number (00-99):
             >= 50 converts to 19XX, < 50 converts to 20XX
        yyyy the year as four digit number
        */
        static Date FromString(const std::string &str, 
                               const std::string &format = "d.M.yyyy"); // can throw

        // Returns succed or failed
        static bool FromString(const std::string &str, 
                               const std::string &format, Date &date); // can throw

    };

    // -----------------------------------------------

    class Time : public ComparableT<Time>
    {
        bool m_isNull;

        word m_msec;
        byte m_sec;
        byte m_min;
        byte m_hour;

        mutable int m_compareValue;  // value for quick compare

        void AssertNotNull() const
        {
            ESS_ASSERT(!m_isNull);
        }

        void ResetCompareValue()
        {
            m_compareValue = -1;
        }

        int ToInteger() const;

    public:

        ESS_TYPEDEF(BadTime);

        Time()
        {
            Clear();
        }

        Time(byte hour, byte min, byte sec, word msec = 0)
        {
            if (!Set(hour, min, sec, msec)) ESS_THROW(BadTime);
        }

        void Clear()
        {
            m_isNull = true;
        }

        bool Set(byte hour, byte min, byte sec, word msec = 0)
        {
            m_hour = hour;
            m_min = min;
            m_sec = sec;
            m_msec = msec;

            m_isNull = false;            

            if (!IsValid()) 
            {
                m_isNull = true;
                return false;
            }

            ResetCompareValue();

            return true;
        }

        bool IsNull() const
        {
            return m_isNull;
        }

        bool IsValid() const;

        int AddTicksToTime(dword ticks);

        word MSec() const   { AssertNotNull(); return m_msec; }
        byte Sec()  const   { AssertNotNull(); return m_sec; }
        byte Min()  const   { AssertNotNull(); return m_min; }
        byte Hour() const   { AssertNotNull(); return m_hour; }

        int Compare(const Time &other) const;

        dword AsSeconds() const;

        static dword SecondsInDay();
        
        ESS_TYPEDEF(InvalidTimeFormat);
        
        /*
        Format:
        h   the hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)
        hh  the hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)
        H   the hour without a leading zero (0 to 23, even with AM/PM display)
        HH  the hour with a leading zero (00 to 23, even with AM/PM display)
        m   the minute without a leading zero (0 to 59)
        mm  the minute with a leading zero (00 to 59)
        s   the second without a leading zero (0 to 59)
        ss  the second with a leading zero (00 to 59)
        z   the milliseconds without leading zeroes (0 to 999)
        zzz the milliseconds with leading zeroes (000 to 999)
        AP  use AM/PM display. AP will be replaced by either "AM" or "PM".
        ap  use am/pm display. ap will be replaced by either "am" or "pm".
        */
        std::string ToString(const std::string &format = "H:mm:ss.zzz") const; // can throw

        /*
        If input string is empty or parsing is failed an invalid Time will be returned.
        For any field that is not represented in the format the following defaults are used:
        Hour = 0, Minute = 0, Second = 0, Millisecond = 0.

        Format: 
            same as for ToString(), except AP indicator have to be always placed after hour.
        */
        static Time FromString(const std::string &str, 
                               const std::string &format = "H:mm:ss.zzz"); // can throw

        // Returns succed or failed
        static bool FromString(const std::string &str, 
                               const std::string &format, Time &time); // can throw

    };

    // -----------------------------------------------

    class DateTime : public ComparableT<DateTime>
    {
        Date m_date;
        Time m_time;

    public:

        DateTime() : ComparableT<DateTime>(*this)
        {
        }

        DateTime(const Date &date, const Time &time) : ComparableT<DateTime>(*this),
            m_date(date),
            m_time(time)
        {
        }


        // low-case name for avoiding conflict with Utils::Date, Utils::Time
        Date& date() { return m_date; }
        const Date& date() const { return m_date; }

        Time& time() { return m_time; }
        const Time& time() const { return m_time; }

        bool IsNull() const
        {
            return (m_date.IsNull()) || (m_time.IsNull());
        }

        void AddTicksToDateTime(dword ticks);
        
        int Compare(const DateTime &other) const;

        std::string ToString(const std::string &dateFormat = "d.M.yyyy", 
                             const std::string &timeFormat = "H:mm:ss.zzz") const;
    };


    // debug
    void DtCaptureVisualCheck();
	
}  // namespace Utils


#endif

