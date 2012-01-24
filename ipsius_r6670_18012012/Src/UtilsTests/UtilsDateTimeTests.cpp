
#include "stdafx.h"
#include "UtilsTests/UtilsDateTimeTests.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/UtilsDateTime.h"


namespace UtilsDateTests
{
    using namespace Utils;
    /*
    Date();
    Date(word year, byte month, byte day);
    void Clear();
    void Set(word year, byte month, byte day);
    bool IsNull() const;
    byte Day() const;
    byte Month() const;
    word Year() const;
    bool IsValid() const;
    */
    void ConstructorsAndAccessTest()
    {
        {
            Date date;
            TUT_ASSERT(date.IsNull());
            TUT_ASSERT(!date.IsValid());
        }
        {
            Date date;
            
            date.Set(2001, 11, 12);
            TUT_ASSERT(!date.IsNull());
            TUT_ASSERT(date.IsValid());
            TUT_ASSERT(date.Year() == 2001);
            TUT_ASSERT(date.Month() == 11);
            TUT_ASSERT(date.Day() == 12);
            
            date.Set(2000, 12, 25);
            TUT_ASSERT(!date.IsNull());
            TUT_ASSERT(date.IsValid());
            TUT_ASSERT(date.Year() == 2000);
            TUT_ASSERT(date.Month() == 12);
            TUT_ASSERT(date.Day() == 25);

            date.Clear();
            date.Clear(); // ok
            TUT_ASSERT(date.IsNull());
        }
        {
            Date date(2007, 07, 07);
            TUT_ASSERT(!date.IsNull());
            TUT_ASSERT(date.IsValid());
            TUT_ASSERT(date.Year() == 2007);
            TUT_ASSERT(date.Month() == 07);
            TUT_ASSERT(date.Day() == 07);
        }
        {
            Date date(1996, 2, 1); // leap year

            dword addDays = 500;
            Date date2(date);
            date2.AddDaysToDate(addDays); // !leapYear

            TUT_ASSERT((date2.AsDays() - date.AsDays()) == addDays);
        }
        {
            Date date(1994, 2, 1); // !leap year

            dword addDays = 500;
            Date date2(date);
            date2.AddDaysToDate(addDays); // !leapYear

            TUT_ASSERT((date2.AsDays() - date.AsDays()) == addDays);
        }
        {
            Date date(1995, 2, 1); // !leap year

            dword addDays = 500;
            Date date2(date);
            date2.AddDaysToDate(addDays); // leapYear

            TUT_ASSERT((date2.AsDays() - date.AsDays()) == addDays);
        }
    }

    // --------------------------------------------
    /*
    int Compare(const Date &other) const;
    */
    void CompareTest()
    {
        Date date;
        Date date2;

        TUT_ASSERT(date.Compare(date2) == 0);

        date2.Set(2004, 5, 24);
        TUT_ASSERT(date.Compare(date2) == -1);
        TUT_ASSERT(date2.Compare(date) == 1);

        date.Set(2005, 5, 2);
        TUT_ASSERT(date.Compare(date2) == 1);
    }

    // --------------------------------------------
    /*
    void AddDaysToDate(dword days);
    */
    void AddDaysTest()
    {
        {
            Date date(1989, 2, 25);
            date.AddDaysToDate(4);
            TUT_ASSERT(date.Compare(Date(1989, 3, 1)) == 0);
            
            date.Set(1996, 2, 25); // leap year
            date.AddDaysToDate(4);
            TUT_ASSERT(date.Compare(Date(1996, 2, 29)) == 0);
        }
        {
            Date date(2005, 12, 31);
            date.AddDaysToDate(1);
            TUT_ASSERT(date.Compare(Date(2006, 1, 1)) == 0);
        }
        {
            Date date(2009, 1, 1);
            date.AddDaysToDate(364);
            TUT_ASSERT(date.Compare(Date(2009, 12, 31)) == 0);
        }
    }

    // --------------------------------------------
    /*
    void NextDay();
    */
    void NextDayTest()
    {
        {
            Date date(1996, 2, 28); // leap year
            date.NextDay();
            TUT_ASSERT(date.Day() == 29);
        }
        {
            Date date(1990, 2, 28);
            date.NextDay();
            TUT_ASSERT(date.Day() == 1);
            date.NextDay();
            TUT_ASSERT(date.Day() == 2);
        }
        {
            Date date(1900, 2, 28); // !leap year
            date.NextDay();
            TUT_ASSERT(date.Day() == 1);
        }
        {
            Date date(2005, 12, 31);
            date.NextDay();
            TUT_ASSERT(date.Day() == 1);
        }
    }

    // --------------------------------------------

    bool FormatExceptionInToString(const std::string &format)
    {
        try
        {
            Date date(2001, 1, 1);
            std::string str = date.ToString(format);
        }
        catch(const Date::InvalidDateFormat &e)
        {
            return true;
        }
        
        return false;
    }

    // --------------------------------------------

    bool FormatExceptionInFromString(const std::string &str, const std::string &format)
    {
        try
        {
            Date date = Date::FromString(str, format);
        }
        catch(const Date::InvalidDateFormat &e)
        {
            return true;
        }
        
        return false;
    }

    // --------------------------------------------
    /*
    std::string ToString(const std::string &format = "d.M.yyyy") const;
    format:
    d, dd
    M - MMMM
    yy, yyyy
    */
    void ToStringTest()
    {
        {
            Date date(2009, 03, 06);
            
            TUT_ASSERT(date.ToString("d.M.yyyy") == "6.3.2009");
            TUT_ASSERT(date.ToString("dd.MM.yyyy") == "06.03.2009");
            TUT_ASSERT(date.ToString("dd MMMM yy") == "06 March 09");
            TUT_ASSERT(date.ToString("d dd") == "6 06");
            TUT_ASSERT(date.ToString("M MM MMM MMMM") == "3 03 Mar March");
            TUT_ASSERT(date.ToString("yyyy yy") == "2009 09");
            TUT_ASSERT(date.ToString("d'd 'M'M 'yyyy'y'") == "6d 3M 2009y");
    
            TUT_ASSERT(date.ToString("d M yyyy MM dd yy MMMM 'asdfffec' ';zxcfyymd") 
                       == "6 3 2009 03 06 09 March asdfffec ;zxcfyymd");

            TUT_ASSERT(date.ToString("") == "");
        }
        {
            Date date(1900, 12, 31);
            TUT_ASSERT(date.ToString("d.M.yyyy") == "31.12.1900");
            TUT_ASSERT(date.ToString("dd.MM.yyyy") == "31.12.1900");
            TUT_ASSERT(date.ToString("d.M.yy") == "31.12.00");
        }
        
        TUT_ASSERT(FormatExceptionInToString("y"));
        TUT_ASSERT(FormatExceptionInToString("yyy"));
        TUT_ASSERT(FormatExceptionInToString("yyyyy"));
        TUT_ASSERT(FormatExceptionInToString("ddd"));
        TUT_ASSERT(FormatExceptionInToString("MMMMM"));
    }

    // --------------------------------------------
    /*
    static Date FromString(const std::string &str, const std::string &format = "d.M.yyyy");
    format:
    d, dd
    M, MM
    yyyy
    */
    void FromStringTest()
    {
        TUT_ASSERT(Date::FromString("6.3.2009", "d.M.yyyy") == Date(2009, 03, 06));
        TUT_ASSERT(Date::FromString("16.12.2009", "d.M.yyyy") == Date(2009, 12, 16));
        TUT_ASSERT(Date::FromString("16.12.09", "d.M.yy") == Date(2009, 12, 16));
        TUT_ASSERT( !Date::FromString("06.3.2009", "d.M.yyyy").IsValid() );
        TUT_ASSERT( !Date::FromString("6.03.2009", "d.M.yyyy").IsValid() );

        // assert: year < 1900
        // Date date = Date::FromString("6.3.0009", "d.M.yyyy"); 
        
        TUT_ASSERT(Date::FromString("06.03.2009", "dd.MM.yyyy") == Date(2009, 03, 06));
        TUT_ASSERT(Date::FromString("06.03.09", "dd.MM.yy") == Date(2009, 03, 06));
        
        TUT_ASSERT(Date::FromString("06.03.00", "dd.MM.yy") == Date(2000, 03, 06));
        TUT_ASSERT(Date::FromString("06.03.49", "dd.MM.yy") == Date(2049, 03, 06));

        TUT_ASSERT(Date::FromString("06.03.50", "dd.MM.yy") == Date(1950, 03, 06));
        TUT_ASSERT(Date::FromString("06.03.99", "dd.MM.yy") == Date(1999, 03, 06));

        TUT_ASSERT(Date::FromString("6 06", "d dd") == Date(1900, 01, 06));
        TUT_ASSERT(Date::FromString("3 03", "M MM") == Date(1900, 03, 01));
        TUT_ASSERT(Date::FromString("09 2009", "yy yyyy") == Date(2009, 01, 01));

        TUT_ASSERT(Date::FromString("6d 3M 2009y", "d'd 'M'M 'yyyy'y'") == Date(2009, 03, 06));
        TUT_ASSERT(Date::FromString("6 3 2005 03 06 2009  asdfffec ;zxcfyymd", 
                                    "d M yyyy MM dd yyyy  'asdfffec' ';zxcfyymd") 
                   == Date(2009, 03, 06));

        TUT_ASSERT( !Date::FromString("", "").IsValid() );

        TUT_ASSERT(FormatExceptionInFromString("11", "ddd"));
        TUT_ASSERT(FormatExceptionInFromString("11", "MMM"));
        TUT_ASSERT(FormatExceptionInFromString("11", "y"));
        TUT_ASSERT(FormatExceptionInFromString("11", "yyy"));
        TUT_ASSERT(FormatExceptionInFromString("2000", "yyyyy"));

        {
            Date checkDate(2001, 12, 12);
            Date date;

            TUT_ASSERT(Date::FromString("2001, 12, 12", "yyyy, M, d", date));
            TUT_ASSERT(date == checkDate);
        }
    }
    
    
} // namespace UtilsDateTests

// -------------------------------------------------------------

namespace UtilsTimeTests
{
    using namespace Utils;
    
    /*
    Time();
    Time(byte hour, byte min, byte sec, word msec = 0);
    void Clear();
    void Set(byte hour, byte min, byte sec, word msec = 0);
    bool IsNull() const;
    bool IsValid() const;
    word MSec() const;
    byte Sec()  const;
    byte Min()  const;
    byte Hour() const;
    */
    void ConstructorsAndAccessTest()
    {
        {
            Time time;
            TUT_ASSERT(time.IsNull());
            TUT_ASSERT(!time.IsValid());
        }
        {
            Time time;
            time.Clear();
            TUT_ASSERT(time.IsNull());
        }
        // check range
        {
            Time time(23, 59, 59, 999);
            time.Set(0,0,0,0);
            // time.Set(24, 0, 0);
            // time.Set(0, 60, 0);
            // time.Set(0, 0, 60);
        }
        {
            Time time(23, 12, 15, 156);
            TUT_ASSERT(!time.IsNull());
            TUT_ASSERT(time.IsValid());
            TUT_ASSERT(time.Hour() == 23);
            TUT_ASSERT(time.Min() == 12);
            TUT_ASSERT(time.Sec() == 15);
            TUT_ASSERT(time.MSec() == 156);

            time.Clear();
            TUT_ASSERT(time.IsNull());
            TUT_ASSERT(!time.IsValid());

            time.Set(12, 5, 0);
            TUT_ASSERT(!time.IsNull());
            TUT_ASSERT(time.IsValid());
            TUT_ASSERT(time.Hour() == 12);
            TUT_ASSERT(time.Min() == 5);
            TUT_ASSERT(time.Sec() == 0);
            TUT_ASSERT(time.MSec() == 0);
        }
        {
            Time time(0, 0, 0, 0);
            dword ticks = 2 * 60 * 60 * 1000 + 25 * 60 * 1000 + 59 * 100 + 128;
            time.AddTicksToTime(ticks);
            TUT_ASSERT(time.AsSeconds() == (ticks / 1000));
        }
    }

    // --------------------------------------------
    /*
    int Compare(const Time &other) const;
    */
    void CompareTest()
    {
        Time time;
        Time time2;

        TUT_ASSERT(time.Compare(time2) == 0);

        time2.Set(15, 26, 59, 156);
        TUT_ASSERT(time.Compare(time2) == -1);
        TUT_ASSERT(time2.Compare(time) == 1);

        time.Set(16, 5, 2);
        TUT_ASSERT(time.Compare(time2) == 1);
    }
    
    // --------------------------------------------
    /*
    int AddTicksToTime(dword ticks);
    */
    void AddTicksToTimeTest()
    {
        {
            Time time(0, 0, 1);
            time.AddTicksToTime(1000); // 1 sec
            TUT_ASSERT(time == Time(0, 0, 2));

            time.AddTicksToTime(60 * 1000); // 1 min
            TUT_ASSERT(time == Time(0, 1, 2));

            time.AddTicksToTime(60 * 60 * 1000); // 1 hour
            TUT_ASSERT(time == Time(1, 1, 2));
        }
        {
            Time time(0, 0, 1);
            dword t = (60 * 60 + 45 * 60 + 5) * 1000; // 1h 45m 5sec
            time.AddTicksToTime(t);
            TUT_ASSERT(time == Time(1, 45, 6));
        }
        {
            Time time(0, 0, 0, 1);
            time.AddTicksToTime(999);
            TUT_ASSERT(time == Time(0, 0, 1, 0));

            time.AddTicksToTime(59 * 1000);
            TUT_ASSERT(time == Time(0, 1, 0, 0));

            time.AddTicksToTime(59 * 60 * 1000);
            TUT_ASSERT(time == Time(1, 0, 0, 0));
        }
    }

    // --------------------------------------------

    bool FormatExceptionInToString(const std::string &format)
    {
        try
        {
            Time time(1, 1, 1);
            std::string str = time.ToString(format);
        }
        catch(const Time::InvalidTimeFormat &e)
        {
            return true;
        }
        
        return false;
    }
    
    // --------------------------------------------

    bool FormatExceptionInFromString(const std::string &str, const std::string &format)
    {
        try
        {
            Time time = Time::FromString(str, format);
        }
        catch(const Time::InvalidTimeFormat &e)
        {
            return true;
        }
        
        return false;
    }
    
    // --------------------------------------------
    
    /*
    std::string ToString(const std::string &format = "H:mm:ss.zzz") const;
    format:
    h, hh
    H, HH
    m, mm
    s, ss
    z, zzz
    ap, AP
    */
    void ToStringTest()
    {
        {
            Time time(4, 5, 6, 85);
            TUT_ASSERT(time.ToString("h:mm:ss.zzz") == "4:05:06.085");
            TUT_ASSERT(time.ToString("h m s z") == "4 5 6 85");
            TUT_ASSERT(time.ToString("hh mm ss zzz") == "04 05 06 085");
            TUT_ASSERT(time.ToString("hh m s z AP") == "04 5 6 85 AM");
            TUT_ASSERT(time.ToString("apzsmh") == "am85654");
            TUT_ASSERT(time.ToString("h'h' m'm' s's' z'z'('ap')") == "4h 5m 6s 85z(ap)");
            TUT_ASSERT(time.ToString("h:mm':ss.zzz") == "4:05:ss.zzz");
        }
        {
            TUT_ASSERT(Time(23, 59, 59, 999).ToString("h:mm:ss.zzz") == "23:59:59.999");
            TUT_ASSERT(Time(0, 0, 0, 0).ToString("h:mm:ss.zzz") == "0:00:00.000");
        }
        {
            Time time(13, 5, 6, 85);
            TUT_ASSERT(time.ToString("h m s z ap") == "1 5 6 85 pm");
            TUT_ASSERT(time.ToString("hh m s z AP") == "01 5 6 85 PM");
        }
        {
            Time time(13, 5, 6, 85);
            TUT_ASSERT(time.ToString("H m s z ap") == "13 5 6 85 pm");
            TUT_ASSERT(time.ToString("HH m s z AP") == "13 5 6 85 PM");
        }

        TUT_ASSERT(Time(12, 0, 0, 0).ToString("ap") == "pm");
        TUT_ASSERT(Time(0, 0, 0, 0).ToString("ap") == "am");

        TUT_ASSERT(FormatExceptionInToString("hhh"));
        TUT_ASSERT(FormatExceptionInToString("HHH"));
        TUT_ASSERT(FormatExceptionInToString("mmm"));
        TUT_ASSERT(FormatExceptionInToString("sss"));
        TUT_ASSERT(FormatExceptionInToString("zzzz"));
    }

    // --------------------------------------------
    /*
    static Time FromString(const std::string &str, const std::string &format = "H:mm:ss.zzz");
    */
    void FromStringTest()
    {
        {
            Time checkTime(4, 5, 6, 85);
            TUT_ASSERT(Time::FromString("4:05:06.085", "h:mm:ss.zzz") == checkTime);
            TUT_ASSERT(Time::FromString("04:05:06.085", "hh:mm:ss.zzz") == checkTime);
            TUT_ASSERT(Time::FromString("4:05:06.085", "H:mm:ss.zzz") == checkTime);
            TUT_ASSERT(Time::FromString("04:05:06.085", "HH:mm:ss.zzz") == checkTime);

            TUT_ASSERT(Time::FromString("4:5:06.085", "h:m:ss.zzz") == checkTime);
            TUT_ASSERT(Time::FromString("4:05:06.085", "h:mm:ss.zzz") == checkTime);
            
            TUT_ASSERT(Time::FromString("4:5:6.085", "h:m:s.zzz") == checkTime);
            TUT_ASSERT(Time::FromString("4:5:06.085", "h:m:ss.zzz") == checkTime);

            TUT_ASSERT(Time::FromString("4:5:6.85", "h:m:s.z") == checkTime);
            TUT_ASSERT(Time::FromString("4:5:6.085", "h:m:s.zzz") == checkTime);
            
            TUT_ASSERT( !Time::FromString("04:5:6.85", "h:m:s.z").IsValid() );
            TUT_ASSERT( !Time::FromString("4:05:6.85", "h:m:s.z").IsValid() );
            TUT_ASSERT( !Time::FromString("4:5:06.85", "h:m:s.z").IsValid() );
            TUT_ASSERT( !Time::FromString("4:5:6.085", "h:m:s.z").IsValid() );
            TUT_ASSERT( !Time::FromString("4:5:6.08", "h:m:s.z").IsValid() );

            TUT_ASSERT(Time::FromString("4 5 6 85", "h m s z") == checkTime);

            TUT_ASSERT(Time::FromString("04 05 06 085", "hh mm ss zzz") == checkTime);
            TUT_ASSERT(Time::FromString("04 5 6 85 AM", "hh m s z AP") == checkTime);

            TUT_ASSERT( !Time::FromString("am85654", "apzsmh").IsValid() );
            
            // am will be parsed before hour but it doesn't change result
            TUT_ASSERT(Time::FromString("am 85 6 5 4", "ap z s m h") == checkTime);
            
            // pm will be parsed before hour: 
            // firstly hour = 12 (pm from default hour == 0), then hour = 4
            TUT_ASSERT(Time::FromString("pm 85 6 5 4", "ap z s m h") != Time(16, 5, 6, 85));

            TUT_ASSERT(Time::FromString("4h 5m 6s 85z(ap)", 
                                        "h'h' m'm' s's' z'z'('ap')") == checkTime);
            
            TUT_ASSERT(Time::FromString("4:05:ss.zzz", "h:mm':ss.zzz") == Time(4, 5, 0, 0));
        }
        {
            Time checkTime(13, 5, 6, 85);
            TUT_ASSERT(Time::FromString("1 5 6 85 pm", "h m s z ap") == checkTime);
            TUT_ASSERT(Time::FromString("01 05 06 085 PM", "hh mm ss zzz AP") == checkTime);
        
            TUT_ASSERT(Time::FromString("13 5 6 85 pm", "H m s z ap") == checkTime);
            TUT_ASSERT(Time::FromString("13 5 6 85 PM", "HH m s z AP") == checkTime);

            // ignore AP
            TUT_ASSERT(Time::FromString("1 5 6 85 pm", "H m s z ap") != checkTime);
            TUT_ASSERT(Time::FromString("1 5 6 85 pm", "H m s z ap") == Time(1, 5, 6, 85));
        }
        {
            Time time(23, 59, 59, 999);
            TUT_ASSERT(Time::FromString("23, 59, 59, 999", "h, m, s, z") == time);
        }

        TUT_ASSERT( !Time::FromString("", "").IsValid() );
        
        TUT_ASSERT(Time::FromString("1", "h") == Time(1, 0, 0, 0));
        TUT_ASSERT(Time::FromString("1", "m") == Time(0, 1, 0, 0));
        TUT_ASSERT(Time::FromString("1", "s") == Time(0, 0, 1, 0));
        TUT_ASSERT(Time::FromString("1", "z") == Time(0, 0, 0, 1));

        TUT_ASSERT(FormatExceptionInFromString("11", "HHH"));
        TUT_ASSERT(FormatExceptionInFromString("11", "hhh"));
        TUT_ASSERT(FormatExceptionInFromString("11", "mmm"));
        TUT_ASSERT(FormatExceptionInFromString("11", "sss"));
        TUT_ASSERT(FormatExceptionInFromString("111", "zzzz"));

        {
            Time checkTime(12, 12, 12, 12);
            Time time;

            TUT_ASSERT(Time::FromString("12, 12, 12, 12", "h, m, s, z", time));
            TUT_ASSERT(time == checkTime);
        }
    }
    
} // namespace UtilsTimeTests

// -------------------------------------------------------------

namespace 
{
    using namespace Utils;
    
    /*
    DateTime();
    Date& date();
    Time& time();
    bool IsNull() const;
    int Compare(const DateTime &other) const;
    */
    void UtilsDateTimeTest()
    {
        {
            DateTime dt;
            TUT_ASSERT(dt.IsNull());
    
            dt.date().Set(2009, 03, 16);
            TUT_ASSERT(dt.IsNull());

            dt.time().Set(0, 0, 0);
            TUT_ASSERT(!dt.IsNull());
        }
        {
            DateTime dt;
            dt.time().Set(0, 0, 0);
            TUT_ASSERT(dt.IsNull());
        }
        {
            TUT_ASSERT(DateTime() == DateTime());
            
            DateTime dt1, dt2;
            dt1.date().Set(2009, 03, 16);
            dt2.date().Set(2009, 03, 17);
            TUT_ASSERT(dt1.Compare(dt2) < 0);

            dt1.time().Set(01, 03, 17);
            dt2.time().Set(01, 03, 16);
            TUT_ASSERT(dt1.Compare(dt2) < 0);
            
            dt1.date().Clear();
            dt2.date().Clear();
            TUT_ASSERT(dt1.Compare(dt2) > 0);
        }
        {
            DateTime dt;
            dt.date().Set(2009, 1, 1);
            dt.time().Set(23, 59, 30);

            // add tick to time - date doesn't changed
            dt.time().AddTicksToTime(2 * 60 * 1000); // 2 min
            TUT_ASSERT(dt.date().Day() == 1);

            // add 2 min
            dt.AddTicksToDateTime(2 * 60 * 1000);
            TUT_ASSERT(dt.date().Day() == 1);
            
            // add one day
            dt.AddTicksToDateTime(24 * 60 * 60 * 1000); // one day
            TUT_ASSERT(dt.date().Day() == 2);
        }
    }
    
    
} // namespace 

// -------------------------------------------------------------

namespace UtilsTests
{
    void UtilsDateTimeTests()
    {
        UtilsDateTests::ConstructorsAndAccessTest();
        UtilsDateTests::CompareTest();
        UtilsDateTests::AddDaysTest();
        UtilsDateTests::NextDayTest();
        UtilsDateTests::ToStringTest();
        UtilsDateTests::FromStringTest();
    
        UtilsTimeTests::ConstructorsAndAccessTest();
        UtilsTimeTests::CompareTest();
        UtilsTimeTests::AddTicksToTimeTest();
        UtilsTimeTests::ToStringTest();

        UtilsTimeTests::FromStringTest();
        
        UtilsDateTimeTest();
    }
    
} // namespace UtilsTests
