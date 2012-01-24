#include "stdafx.h"
#include "Utils/UtilsDateTime.h"
#include "Utils/DateTimeCapture.h"
#include "Utils/StringUtils.h"
#include "Utils/IntToString.h"

using Platform::word;

namespace
{
    // using Utils::IntToString
    /*
    std::string IntToStr(int i)
    {
        std::ostringstream ss;
        ss << i;
        return ss.str();
    }
    

    std::string PadInteger(int value, int length)
    {
        return Utils::PadLeft( IntToStr(value), length, '0');
    }
    */

    int IntCompare(int left, int right)
    {
        if (left == right) return 0;
        return (left > right) ? 1 : -1;
    }

    // --------------------------------------------------------------------
    // Common for Date and Time parsers helpers 
    
    int CountCharInStringInSeries(char ch, const std::string &str, size_t startPos = 0)
    {
        int count = 0;
        while ((startPos < str.size()) && (str.at(startPos++) == ch)) ++count;
        
        return count;
    }

    // --------------------------------------------------------------------

    bool ExtractNumber(const std::string &str, size_t pos, int reqDigitCount, 
                       int maxDigitCount, int &num, int &digitCount)
    {
        std::string numStr;
        while ((pos < str.size()) && (isdigit(str.at(pos))))
        {
            numStr += str.at(pos);
            ++pos;
        }

        if ((numStr.size() == 0) || (numStr.size() > maxDigitCount)) return false;

        // if format 'x' and value 09 -- error
        // if format 'x' and value 9 -- ok
        // if format 'xx' and value 09 -- ok
        if ((numStr.at(0) == '0') && (numStr.size() > reqDigitCount) ) return false;
        
        digitCount = numStr.size();
        
        if (!Utils::StringToInt(numStr, num)) return false;

        return true;
    }
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------

namespace
{
    const int CMsInSec          = 1000;
    const int CSecInMinute      = 60;
    const int CMinutesInHour    = 60;
    const int CHoursInDay       = 24;
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------

// Time::To/FromString() helpers
namespace 
{
    using namespace Utils;
    
    const char CHourCharIgnoreAp = 'H';
    const char CHourChar = 'h';
    const char CMinuteChar = 'm';
    const char CSecChar = 's';
    const char CMilliSecChar = 'z';

    const std::string CAmPmLow = "ap";
    const std::string CAmPmHi = "AP";

    const std::string CAmLow = "am";
    const std::string CAmHi = "AM";
    const std::string CPmLow = "pm";
    const std::string CPmHi = "PM";

    // --------------------------------------------------------------------
    /*
    format:
    h  the hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)
    hh  the hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)
    H  the hour without a leading zero (0 to 23, even with AM/PM display)
    HH  the hour with a leading zero (00 to 23, even with AM/PM display)
    */
    std::string HourToString(int hour, int hourCharCount, bool inAmPmFormat)
    {
        ESS_ASSERT((hour >= 0) && (hour < CHoursInDay));

        if ((inAmPmFormat) && (hour > 12)) hour -= 12;
        
        switch (hourCharCount)
        {
        case 1: return IntToString(hour);
        case 2: return IntToString(hour, 2); 
        default: ESS_THROW_MSG(Time::InvalidTimeFormat, std::string(hourCharCount, CHourChar));
        }

        return "";
    }

    // --------------------------------------------------------------------
    
    bool HourFromString(const std::string &str, size_t strStartPos, 
                        int hourCharCount, byte &hour, int &digitCount)
    {
        const int maxDigitCount = 2;
        
        // check hourCharCount
        if ((hourCharCount < 1) || (hourCharCount > maxDigitCount))
        {
            ESS_THROW_MSG(Time::InvalidTimeFormat, std::string(hourCharCount, CHourChar));
        }

        int tmp = 0;
        if (!ExtractNumber(str, strStartPos, hourCharCount, maxDigitCount, tmp, digitCount))
        {
            return false;
        }

        hour = tmp;
        
        return true;
    }

    // --------------------------------------------------------------------
    /*
    format:
    m  the minute without a leading zero (0 to 59)
    mm  the minute with a leading zero (00 to 59)
    */
    std::string MinuteToString(int minute, int minuteCharCount)
    {
        ESS_ASSERT((minute >= 0) && (minute < CMinutesInHour));
        switch (minuteCharCount)
        {
        case 1: return IntToString(minute);
        case 2: return IntToString(minute, 2); 
        default: ESS_THROW_MSG(Time::InvalidTimeFormat, std::string(minuteCharCount, CMinuteChar));
        }

        return "";
    }

    // --------------------------------------------------------------------

    bool MinuteFromString(const std::string &str, size_t strStartPos, 
                          int minuteCharCount, byte &minute, int &digitCount)
    {
        const int maxDigitCount = 2;
        if ((minuteCharCount < 1) || (minuteCharCount > maxDigitCount))
        {
            ESS_THROW_MSG(Time::InvalidTimeFormat, std::string(minuteCharCount, CMinuteChar));
        }

        int tmp = 0;
        if (!ExtractNumber(str, strStartPos, minuteCharCount, maxDigitCount, tmp, digitCount))
        {
            return false;
        }

        minute = tmp;
        
        return true;
    }

    // --------------------------------------------------------------------
    /*
    format:
    s  the second without a leading zero (0 to 59)
    ss  the second with a leading zero (00 to 59)
    */
    std::string SecToString(int sec, int secCharCount)
    {
        ESS_ASSERT((sec >= 0) && (sec < CSecInMinute));
        switch (secCharCount)
        {
        case 1: return IntToString(sec);
        case 2: return IntToString(sec, 2);
        default: ESS_THROW_MSG(Time::InvalidTimeFormat, std::string(secCharCount, CSecChar));
        }

        return "";
    }

    // --------------------------------------------------------------------
    
    bool SecFromString(const std::string &str, size_t strStartPos, 
                       int secCharCount, byte &sec, int &digitCount)
    {
        const int maxDigitCount = 2;
            
        if ((secCharCount < 1) || (secCharCount > maxDigitCount))
        {
            ESS_THROW_MSG(Time::InvalidTimeFormat, std::string(secCharCount, CSecChar));
        }

        int tmp = 0;
        if (!ExtractNumber(str, strStartPos, secCharCount, maxDigitCount, tmp, digitCount))
        {
            return false;
        }

        sec = tmp;
        
        return true;
    }

    // --------------------------------------------------------------------
    /*
    format:
    z  the milliseconds without leading zeroes (0 to 999)
    zzz  the milliseconds with leading zeroes (000 to 999)
    */
    std::string MSecToString(int msec, int msecCharCount)
    {
        ESS_ASSERT((msec >= 0) && (msec < CMsInSec));
        
        switch (msecCharCount)
        {
        case 1: return IntToString(msec);
        case 3: return IntToString(msec, 3);
        default: ESS_THROW_MSG(Time::InvalidTimeFormat, std::string(msecCharCount, CMilliSecChar));
        }

        return "";
    }

    // --------------------------------------------------------------------
    
    bool MSecFromString(const std::string &str, size_t strStartPos, 
                        int msecCharCount, word &msec, int &digitCount)
    {
        const int maxDigitCount = 3;
        
        if ((msecCharCount < 1) || (msecCharCount > maxDigitCount))
        {
            ESS_THROW_MSG(Time::InvalidTimeFormat, std::string(msecCharCount, CMilliSecChar));
        }

        int tmp = 0;
        if (!ExtractNumber(str, strStartPos, msecCharCount, maxDigitCount, tmp, digitCount))
        {
            return false;
        }

        msec = tmp;
        
        return true;
    }
    
    // --------------------------------------------------------------------
    /*
    AP  use AM/PM display. AP will be replaced by either "AM" or "PM".
    ap  use am/pm display. ap will be replaced by either "am" or "pm".
    */
    std::string APToString(bool isAM, char chA, char chP) 
    {
        std::string format;
        format += chA;
        format += chP;
        
        if (format == CAmPmLow) return (isAM)? CAmLow : CPmLow;
        if (format == CAmPmHi) return (isAM)? CAmHi : CPmHi;

        ESS_THROW_MSG(Time::InvalidTimeFormat, format);

        return ""; // never happend
    }

    // --------------------------------------------------------------------

    bool HasAmPmIndicator(const std::string &str)
    {
        for (size_t i = 0; i < str.size() - 1; ++i)
        {
            std::string apPair;
            apPair += str.at(i);
            apPair += str.at(i + 1);
            
            if ((apPair == CAmPmHi) || (apPair == CAmPmLow)) return true;
        }

        return false;
    }

    // --------------------------------------------------------------------

    bool IsAm(byte hour)
    {
        return (hour < 12)? true : false;
    }

    // --------------------------------------------------------------------

    bool IsApFormat(const std::string &input, size_t inputPos, const std::string &format)
    {
        if ((inputPos + format.size()) > input.size()) return false;

        for (size_t i = 0; i < format.size(); ++i)
        {
            if (input.at(inputPos) != format.at(i)) return false;
            ++inputPos;
        }

        return true;
    }
    
} // namespace 

// --------------------------------------------------------------------
// --------------------------------------------------------------------

namespace Utils
{
    bool Time::IsValid() const
    {
        if (m_isNull) return false;
        
        return ((m_hour < CHoursInDay) 
                && (m_min < CMinutesInHour)
                && (m_sec < CSecInMinute)
                && (m_msec < CMsInSec));
                
    }

    // --------------------------------------------------------------------
	
    int Time::AddTicksToTime( dword ticks )
    {
        AssertNotNull();
            
        ResetCompareValue();

        m_msec += ticks % CMsInSec;
        int sec = ticks / CMsInSec;
        if (m_msec >= CMsInSec)
        {
            m_msec -= CMsInSec;
            sec++;
        }

        m_sec += sec % CSecInMinute;
        int minutes = sec / CSecInMinute;
        if (m_sec >= CSecInMinute)
        {
            m_sec -= CSecInMinute;
            minutes++;
        }

        m_min += minutes % CMinutesInHour;
        int hours = minutes / CMinutesInHour;
        if (m_min >= CMinutesInHour)
        {
            m_min -= CMinutesInHour;
            hours++;
        }

        m_hour += hours % CHoursInDay;
        int days = hours / CHoursInDay;
        if (m_hour >= CHoursInDay)
        {
            m_hour -= CHoursInDay;
            days++;
        }

        return days;
    }

    // --------------------------------------------------------------------
    
    // std::string Time::ToString( const std::string &format /*= "H:mm:ss.zzz"*/ ) const
    /*{
        AssertNotNull();

        std::ostringstream ss;
        ss << (int)m_hour << ":" << PadInteger(m_min, 2) << ":" 
            << PadInteger(m_sec, 2) << "." << PadInteger(m_msec, 3);

        return ss.str();
    }
    */

    std::string Time::ToString( const std::string &format /*= "H:mm:ss.zzz"*/ ) const
    {
        AssertNotNull();

        std::string res;

        bool hasAmPmFormat = HasAmPmIndicator(format);

        size_t i = 0;
        while (i < format.size())
        {
            // skip parsing all inside '' and '
            if (format.at(i) == '\'')
            {
                for (++i; i < format.size(); ++i)
                {
                    if (format.at(i) == '\'') break;
                    res += format.at(i);
                }
                ++i; // skip '
                continue;
            }
            
            // hour
            if ((format.at(i) == CHourChar) || (format.at(i) == CHourCharIgnoreAp))
            {
                int count = CountCharInStringInSeries(format.at(i), format, i);

                bool useAmPmFormat = (format.at(i) == CHourCharIgnoreAp)? 
                                     false : hasAmPmFormat;
                
                res += HourToString(m_hour, count, useAmPmFormat);
                i += count;
                continue;
            }

            // min
            if (format.at(i) == CMinuteChar)
            {
                int count = CountCharInStringInSeries(format.at(i), format, i);
                res += MinuteToString(m_min, count);
                i += count;
                continue;
            }

            // sec
            if (format.at(i) == CSecChar)
            {
                int count = CountCharInStringInSeries(format.at(i), format, i);
                res += SecToString(m_sec, count);
                i += count;
                continue;
            }

            // msec
            if (format.at(i) == CMilliSecChar)
            {
                int count = CountCharInStringInSeries(format.at(i), format, i);
                res += MSecToString(m_msec, count);
                i += count;
                continue;
            }

            // am/pm
            if (IsApFormat(format, i, CAmPmHi) || IsApFormat(format, i, CAmPmLow))
            {
                res += APToString(IsAm(m_hour), format.at(i), format.at(i + 1));
                i += 2;
                continue;
            }

            res += format.at(i++);
        }
                
        return res;
    }

    // --------------------------------------------------------------------

    int Time::ToInteger() const
    {
        if (m_compareValue > 0) return m_compareValue;

        {
            int val = m_sec + (m_min * CSecInMinute) + (m_hour * CSecInMinute * CMinutesInHour);
            val = (val * 1000) + m_msec;
            m_compareValue = val;
        }

        return m_compareValue;
    }

    // --------------------------------------------------------------------
    
    int Time::Compare( const Time &other ) const
    {
        // null check
        if (IsNull() || other.IsNull())
        {
            if (IsNull() == other.IsNull()) return 0;
            return (IsNull()) ? -1 : 1;
        }

        // data fields
        return IntCompare(ToInteger(), other.ToInteger());
    }

    // --------------------------------------------------------------------

    dword Time::AsSeconds() const
    {
        AssertNotNull();

        return (m_hour * CMinutesInHour * CSecInMinute + m_min * CSecInMinute + m_sec); 
    }

    // --------------------------------------------------------------------

    dword Time::SecondsInDay()
    {
        return CHoursInDay * CMinutesInHour * CSecInMinute;
    }
    
    // --------------------------------------------------------------------

    Time Time::FromString(const std::string &str, const std::string &format)
    {
        if (str.size() == 0) return Time();
        
        byte hour = 0;
        byte minute = 0;
        byte sec = 0;
        word msec = 0;

        size_t formatInx = 0;
        size_t strInx = 0;
        bool ignoreAP = false;

        while (formatInx < format.size())
        {
            // skip all inside '' and '
            if (format.at(formatInx) == '\'')
            {
                size_t i = formatInx + 1;
                while ((i < format.size()) && (format.at(i++) != '\''));

                strInx += (i - formatInx - 2); // 2 -- two qoutes
                formatInx = i;
                continue;
            }

            // hour
            if ((format.at(formatInx) == CHourChar) 
                || (format.at(formatInx) == CHourCharIgnoreAp))
            {
                if (format.at(formatInx) == CHourCharIgnoreAp) ignoreAP = true;
                
                int count = CountCharInStringInSeries(format.at(formatInx), format, formatInx);
                int digitCount = 0;
                
                if (!HourFromString(str, strInx, count, hour, digitCount)) return Time();

                formatInx += count;
                strInx += digitCount;
                continue;
            }

            // min
            if (format.at(formatInx) == CMinuteChar)
            {
                int count = CountCharInStringInSeries(format.at(formatInx), format, formatInx);
                int digitCount = 0;
                
                if (!MinuteFromString(str, strInx, count, minute, digitCount)) return Time();

                formatInx += count;
                strInx += digitCount;
                continue;
            }

            // sec
            if (format.at(formatInx) == CSecChar)
            {
                int count = CountCharInStringInSeries(format.at(formatInx), format, formatInx);
                int digitCount = 0;
                if (!SecFromString(str, strInx, count, sec, digitCount)) return Time();

                formatInx += count;
                strInx += digitCount;
                continue;
            }

            // msec
            if (format.at(formatInx) == CMilliSecChar)
            {
                int count = CountCharInStringInSeries(format.at(formatInx), format, formatInx);
                int digitCount = 0;
                
                if (!MSecFromString(str, strInx, count, msec, digitCount)) return Time();

                formatInx += count;
                strInx += digitCount;
                continue;
            }

            // am/pm
            if (IsApFormat(format, formatInx, CAmPmHi) 
                || IsApFormat(format, formatInx, CAmPmLow))
            {
                std::string ap;
                ap += str.at(strInx);
                ap += str.at(strInx + 1);
                
                if (!ignoreAP)
                {
                    byte halfDayHours = CHoursInDay / 2;
                    if ((hour > (halfDayHours - 1)) 
                        && ((ap == CAmLow) || (ap == CAmHi))) return Time();
                    
                    if ((hour < halfDayHours) 
                        && ((ap == CPmLow) || (ap == CPmHi))) hour += halfDayHours;
                }
                
                formatInx += 2;
                strInx += 2; 
                continue;
            }
            
            ++formatInx;
            ++strInx;
        }
                
        return Time(hour, minute, sec, msec);
    }

    // --------------------------------------------------------------------

    bool Time::FromString(const std::string &str, const std::string &format, Time &time)
    {
        time = Time::FromString(str, format);

        return time.IsValid();
    }

}  // namespace Utils

// --------------------------------------------------------------------
// --------------------------------------------------------------------

namespace
{
    const int CMonthInYear      = 12;
    const int CMaxDayInMonth    = 31;

    // int DaysInMonth[CMonthInYear]     = {31, 28, 31, 30, 31, 30,   31, 31, 30, 31, 30, 31};
    // int DaysInMonthLeap[CMonthInYear] = {31, 29, 31, 30, 31, 30,   31, 31, 30, 31, 30, 31};

    // --------------------------------------------------------------------

    class MonthInfoTable
    {
        struct MonthInfo
        {
            int DaysInMonth;
            int DaysInMonthLeap;
            std::string ShortName;
            std::string LongName;

            MonthInfo(int daysInMonth, int daysInMonthLeap, 
                      const std::string &shortName, const std::string &longName) :
                DaysInMonth(daysInMonth), DaysInMonthLeap(daysInMonthLeap), 
                ShortName(shortName), LongName(longName)
            {}
        };

        std::vector<MonthInfo> m_list;

        bool ValidMonthNo(int number) const
        {
            return ((number > 0) && (number <= 12));
        }

        static bool YearIsLeap(word year)
        {
            // return ((year % 4) == 0);
            
            bool res = ((year % 4) == 0);
    
            if (((year % 100) == 0) && ((year % 400) != 0)) res = false;
    
            return res;
        }
        
    public:
        MonthInfoTable()
        {
            m_list.push_back(MonthInfo(31, 31, "Jan", "January"));
            m_list.push_back(MonthInfo(28, 29, "Feb", "February"));
            m_list.push_back(MonthInfo(31, 31, "Mar", "March"));
            m_list.push_back(MonthInfo(30, 30, "Apr", "April"));
            m_list.push_back(MonthInfo(31, 31, "May", "May"));
            m_list.push_back(MonthInfo(30, 30, "Jun", "June"));
            
            m_list.push_back(MonthInfo(31, 31, "Jul", "July"));
            m_list.push_back(MonthInfo(31, 31, "Aug", "August"));
            m_list.push_back(MonthInfo(30, 30, "Sep", "September"));
            m_list.push_back(MonthInfo(31, 31, "Oct", "October"));
            m_list.push_back(MonthInfo(30, 30, "Nov", "November"));
            m_list.push_back(MonthInfo(31, 31, "Dec", "December"));
        }

        // monthNo = [1..12]

        std::string ShortName(int monthNo) const
        {
            if (!ValidMonthNo(monthNo)) return std::string(); // can have invalid input monthNo
            
            return m_list.at(monthNo - 1).ShortName;
        }

        std::string LongName(int monthNo) const
        {
            if (!ValidMonthNo(monthNo)) return std::string(); // can have invalid input monthNo
            
            return m_list.at(monthNo - 1).LongName;
        }

        // name - short or long name
        // returns -1 if !found
        int MonthNo(const std::string &name) const
        {
            for (size_t i = 0; i < m_list.size(); ++i)
            {
                if ((m_list.at(i).ShortName == name) || (m_list.at(i).LongName == name))
                {
                    return (i + 1);
                }
            }
            
            return -1;
        }

        int DaysInMonth(int monthNo, word year) const
        {
            ESS_ASSERT(ValidMonthNo(monthNo)); // can't have invalid input monthNo

            int index = monthNo - 1;
            return (YearIsLeap(year) ? 
                    m_list.at(index).DaysInMonthLeap : m_list.at(index).DaysInMonth);
        }
    };

    const MonthInfoTable CMonthInfoTable = MonthInfoTable();

    // --------------------------------------------------------------------
    // for ToFromString  parsing
    
    using namespace Utils;

    const char CDayChar = 'd';
    const char CMonthChar = 'M';
    const char CYearChar = 'y';

    // for FromString() parsing
    word CDefaultYear = 1900; 
    byte CDefaultMonth = 1;
    byte CDefaultDay = 1;
        
    /*
    format:
    d    the day as number without a leading zero (1 to 31);
    dd   the day as number with a leading zero (01 to 31;
    */
    std::string DayToString(int day, int dayCharCount)
    {
        ESS_ASSERT((day > 0) && (day <= CMaxDayInMonth));

        switch (dayCharCount)
        {
        case 1: return IntToString(day); // "d"
        case 2: return IntToString(day, 2); // "dd"
        default:
            ESS_THROW_MSG(Date::InvalidDateFormat, std::string(dayCharCount, CDayChar));
        }

        return "";
    }

    // --------------------------------------------------------------------

    bool DayFromString(const std::string &str, size_t strStartPos, int dayCharCount, 
                       byte &day, int &digitCount)
    {
        const int maxDigitCount = 2;
        
        if ((dayCharCount < 1) || (dayCharCount > maxDigitCount))
        {
            ESS_THROW_MSG(Date::InvalidDateFormat, std::string(dayCharCount, CDayChar));
        }
       
        int tmp = 0;
        if (!ExtractNumber(str, strStartPos, dayCharCount, maxDigitCount, tmp, digitCount))
        {
            return false;
        }
        
        day = tmp;
        
        return true;
    }

    // --------------------------------------------------------------------
    /*
    format:
    M     the month as number without a leading zero (1-12)
    MM    the month as number with a leading zero (01-12)
    MMM   QDate::shortMonthName().
    MMMM  QDate::longMonthName().
    */
    std::string MonthToString(int month, int monthCharCount)
    {
        ESS_ASSERT((month > 0) && (month <= CMonthInYear));

        switch (monthCharCount)
        {
        case 1: return IntToString(month);
        case 2: return IntToString(month, 2);
        case 3: return CMonthInfoTable.ShortName(month);
        case 4: return CMonthInfoTable.LongName(month);
        default: ESS_THROW_MSG(Date::InvalidDateFormat, std::string(monthCharCount, CMonthChar));
        }

        return "";
    }

    // --------------------------------------------------------------------

    // Format: only M and MM
    bool MonthFromString(const std::string &str, size_t strStartPos, int monthCharCount, 
                         byte &month, int &digitCount)
    {
        const int maxDigitCount = 2;
        
        if ((monthCharCount < 1) || (monthCharCount > maxDigitCount))
        {
            ESS_THROW_MSG(Date::InvalidDateFormat, std::string(monthCharCount, CMonthChar));
        }
       
        int tmp = 0;
        if (!ExtractNumber(str, strStartPos, monthCharCount, maxDigitCount, tmp, digitCount))
        {
            return false;
        }

        month = tmp;
        return true;
    }

    // --------------------------------------------------------------------
    /*
    format:
    yy  the year as two digit number (00-99)
    yyyy  the year as four digit number
    */
    std::string YearToString(int year, int yearCharCount)
    {
        switch (yearCharCount)
        {
        case 2: 
            {
                std::string tmp(IntToString(year, 2));
                return std::string(tmp, tmp.size() - 2, 2);
            }
        case 4: return IntToString(year, 4);
        default: 
            ESS_THROW_MSG(Date::InvalidDateFormat, std::string(yearCharCount, CYearChar));
        }

        return "";
    }

    // --------------------------------------------------------------------

    bool YearFromString(const std::string &str, size_t strStartPos, int yearCharCount, 
                        word &year, int &digitCount)
    {
        const int maxDigitCount = 4;

        if ((yearCharCount != 2) && (yearCharCount != maxDigitCount))
        {
            ESS_THROW_MSG(Date::InvalidDateFormat, std::string(yearCharCount, CYearChar));
        }

        int tmp = 0;
        if (!ExtractNumber(str, strStartPos, yearCharCount, maxDigitCount, tmp, digitCount))
        {
            return false;
        }

        year = tmp;
        
        if (yearCharCount == 2) year = (year >= 50) ? (1900 + year) : (2000 + year);

        ESS_ASSERT(year >= CDefaultYear);
       
        return true;
    }
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------

namespace Utils
{
    bool Date::IsValid() const
    {
        if (m_isNull) return false;
        
        return ((m_month > 0) && (m_month <= CMonthInYear)
                && (m_day > 0) && (m_day <= CMonthInfoTable.DaysInMonth(m_month, m_year)));
    }
    
    // --------------------------------------------------------------------

    void Date::NextDay()
    {
        AssertNotNull();
        
        ResetCompareValue();

        // inc day
        m_day++;

        if (m_day <= CMonthInfoTable.DaysInMonth(m_month, m_year)) return;

        // inc month, reset day
        m_month++;
        m_day = 1;

        if (m_month <= CMonthInYear) return;

        // inc year, reset month and day
        m_year++;
        m_month = 1;
        m_day = 1;
        
        // 0xFFFF year problem? :)
        // ...
    }

    // --------------------------------------------------------------------
    
    void Date::AddDaysToDate(dword days)
    {
        while(days--)
        {
            NextDay();
        }
    }

    // --------------------------------------------------------------------

    int Date::Compare( const Date &other ) const
    {
        // null check
        if (IsNull() || other.IsNull())
        {
            if (IsNull() == other.IsNull()) return 0;
            return (IsNull()) ? -1 : 1;
        }

        // data fields
        return IntCompare(ToInteger(), other.ToInteger());
    }

    // --------------------------------------------------------------------
    // Days since 1 AD
    dword Date::AsDays() const
    {
        AssertNotNull();

        word interval = m_year - 1; // years except last
        
        word leapYearCount = interval / 4 - interval / 100 + interval / 400;

        word daysInYears = m_year * 365 + leapYearCount;

        word daysInMonthes = 0;
        for (int i = 1; i < m_month; ++i)
        {
            daysInMonthes += CMonthInfoTable.DaysInMonth(i, m_year);
        }

        return daysInYears + daysInMonthes + m_day;

    }

    // --------------------------------------------------------------------
    
    std::string Date::ToString( const std::string &format /*= "d.M.yyyy"*/ ) const
    {
        AssertNotNull();

        std::string res;
        size_t i = 0;
        while (i < format.size())
        {
            // skip parsing all inside '' and '
            if (format.at(i) == '\'')
            {
                for (++i; i < format.size(); ++i)
                {
                    if (format.at(i) == '\'') break;
                    res += format.at(i);
                }
                ++i; // skip '
                continue;
            }

            // day
            if (format.at(i) == CDayChar)
            {
                int count = CountCharInStringInSeries(format.at(i), format, i);
                res += DayToString(m_day, count);
                i += count;
                continue;
            }

            // month
            if (format.at(i) == CMonthChar)
            {
                int count = CountCharInStringInSeries(format.at(i), format, i);
                res += MonthToString(m_month, count);
                i += count;
                continue;
            }

            // year
            if (format.at(i) == CYearChar)
            {
                int count = CountCharInStringInSeries(format.at(i), format, i);
                res += YearToString(m_year, count);
                i += count;
                continue;
            }

            res += format.at(i++);
        }
                
        return res;
    }

    // --------------------------------------------------------------------

    Date Date::FromString(const std::string &str, const std::string &format)
    {
        if (str.size() == 0) return Date();

        word year = CDefaultYear; 
        byte month = CDefaultMonth;
        byte day = CDefaultDay;
        
        size_t formatInx = 0;
        size_t strInx = 0;
        while (formatInx < format.size())
        {
            // skip all inside '' and '
            if (format.at(formatInx) == '\'')
            {
                size_t i = formatInx + 1;
                while ((i < format.size()) && (format.at(i++) != '\''));

                strInx += (i - formatInx - 2); // 2 -- two qoutes
                formatInx = i;
                continue;
            }

            // day
            if (format.at(formatInx) == CDayChar)
            {
                int count = CountCharInStringInSeries(format.at(formatInx), format, formatInx);
                int digitCount = 0;
                if (!DayFromString(str, strInx, count, day, digitCount)) return Date(); // invalid date

                formatInx += count;
                strInx += digitCount;
                continue;
            }

            // month 
            if (format.at(formatInx) == CMonthChar)
            {
                int count = CountCharInStringInSeries(format.at(formatInx), format, formatInx);
                int digitCount = 0;
                
                if (!MonthFromString(str, strInx, count, month, digitCount)) return Date();

                formatInx += count;
                strInx += digitCount;
                continue;
            }
            
            // year
            if (format.at(formatInx) == CYearChar)
            {
                int count = CountCharInStringInSeries(format.at(formatInx), format, formatInx);
                int digitCount = 0;
                
                if (!YearFromString(str, strInx, count, year, digitCount)) return Date();
                
                formatInx += count;
                strInx += digitCount;
                continue;
            }
            
            ++formatInx;
            ++strInx;
        }

        return Date(year, month, day);
    }

    // --------------------------------------------------------------------

    bool Date::FromString(const std::string &str, const std::string &format, Date &date)
    {
        date = Date::FromString(str, format);
        
        return date.IsValid();
    }

    
}  // namespace Utils

// --------------------------------------------------------------------
// --------------------------------------------------------------------

namespace Utils
{
    void DateTime::AddTicksToDateTime(dword ticks)
    {
        ESS_ASSERT(!IsNull());
        
        int days = m_time.AddTicksToTime(ticks);
        m_date.AddDaysToDate(days);
    }

    // --------------------------------------------------------------------
    
    int DateTime::Compare( const DateTime &other ) const
    {
        if (m_date > other.m_date) return 1;
        if (m_date < other.m_date) return -1;

        if (m_time > other.m_time) return 1;
        if (m_time < other.m_time) return -1;

        return 0;
    }

    // --------------------------------------------------------------------

    std::string DateTime::ToString(const std::string &dateFormat, 
                                   const std::string &timeFormat) const
    {
        ESS_ASSERT(!IsNull());
        
        return m_date.ToString(dateFormat) + " " + m_time.ToString(timeFormat);
    }

    // --------------------------------------------------------------------

    void DtCaptureVisualCheck()
    {
        int CInerations = 32;
        int CInterval = 200; // 5 * 1000;

        int count = CInerations;

        while(count--)
        {
            DateTimeCapture dt;
            dt.Capture();
            std::cout << dt.date().ToString() << " " << dt.time().ToString() << std::endl; // "      ";
            // std::cout.flush();

            Platform::ThreadSleep(CInterval);
        }
    }

}  // namespace Utils

