
#include "stdafx.h"
#include "UtilsTests/DateTimeCaptureTests.h"
#include "Utils/MsDateTime.h"
#include "Platform/PlatformThread.h"
#include "Platform/PlatformMutex.h"

namespace 
{
    using namespace Utils;

    const dword CMTTestTime = 1000; 
    
    /*
    MsDateTime()
    void Clear()
    bool IsNull() const
    const Date& date() const
    const Time& time() const
    const DateTime& DateAndTime() const
    int Compare(const MsDateTime &other) const
    */
    void InterfaceTest()
    {
        // check constructor
        MsDateTime dt;
        MsDateTime dt2;
        TUT_ASSERT(dt.IsNull());
        TUT_ASSERT(dt2.IsNull());

        // check compare
        TUT_ASSERT(dt.Compare(dt2) == 0);
        dt2.Capture();
        TUT_ASSERT(dt.Compare(dt2) < 0);
        TUT_ASSERT(dt2.Compare(dt) > 0);
        

        // check DateTime !null after capturing
        dt.Capture();
        TUT_ASSERT(!dt.IsNull());
        TUT_ASSERT(!dt.date().IsNull());
        TUT_ASSERT(!dt.time().IsNull());
        TUT_ASSERT(!dt.DateAndTime().IsNull());

        // check DateAndTime(), date(), time() returns same info
        TUT_ASSERT(dt.date() == dt.DateAndTime().date());
        TUT_ASSERT(dt.time() == dt.DateAndTime().time());

        // check clear
        dt.Clear();
        TUT_ASSERT(dt.IsNull());
    }

    // ---------------------------------------------------

    DateTime CreateDT(const Date &date, const Time &time)
    {
        DateTime dt;
        dt.date() = date; 
        dt.time() = time; 

        return dt;
    }
    
    // ---------------------------------------------------

    bool CheckDateTimeInRange(const MsDateTime &toCheck, const Date &minDate, 
                              const Time &minTime, dword rangeMS)
    {
        DateTime minDT(CreateDT(minDate, minTime));
        DateTime maxDT(minDT);
        maxDT.AddTicksToDateTime(rangeMS);

        // std::cout << "min -- " << minDT.ToString() << std::endl;
        // std::cout << "max -- " << maxDT.ToString() << std::endl;

        return ((toCheck.DateAndTime() >= minDT) && (toCheck.DateAndTime() <= maxDT));
    }
    
    
    // ---------------------------------------------------
    /*
    static void SetSyncPoint(const DateTime &datetime); 
    */
    void SetSyncPointTest(const DateTime &dtPoint)
    {
        MsDateTime dt;
        dt.Capture();
        
        MsDateTime::SetSyncPoint(dtPoint);

        dt.Capture();

        dword checkRangeMs = 2 * 1000; 
        TUT_ASSERT(CheckDateTimeInRange(dt, dtPoint.date(), dtPoint.time(), checkRangeMs));
    }
    
    // ---------------------------------------------------
    /*
    Utils::Detail::LongCounter
    */
    void LongCounterTest()
    {
        Detail::LongCounter counter;
        counter.lo = 0xD5E6F718;
        counter.hi = 0xA1B2C3E4;
        Platform::ddword checkVal = 0xA1B2C3E4D5E6F718LL;
        
        Platform::ddword val;
        counter.SetToDDword(val);

        TUT_ASSERT(val == checkVal);
    }
    
    // ---------------------------------------------------
    /*
    void Capture()
    */
    void CaptureTest()
    {
        MsDateTime dt;
        Date date(2009, 3, 19);
        dword checkRangeMs = 2 * 1000;
        
        {
            // set point
            Time time(0, 1, 2);
            MsDateTime::SetSyncPoint(CreateDT(date, time));

            // get time and check
            dt.Capture();
            TUT_ASSERT(CheckDateTimeInRange(dt, date, time, checkRangeMs));
            
            // reset and capture
            dt.Clear();
            dt.Capture();
            TUT_ASSERT(CheckDateTimeInRange(dt, date, time, checkRangeMs));
        }
        {
            Time lastTimePoint(18, 25, 35, 100);
            MsDateTime::SetSyncPoint(CreateDT(date, Time(0, 0, 0, 0)));
            MsDateTime::SetSyncPoint(CreateDT(date, Time(12, 15, 14)));
            MsDateTime::SetSyncPoint(CreateDT(date, lastTimePoint));

            // check that use last set point
            dt.Capture();
            TUT_ASSERT(CheckDateTimeInRange(dt, date, lastTimePoint, checkRangeMs));
        }
    }

    // ---------------------------------------------------

    class SyncPointAdder
    {
        dword m_pointDiffMs;
        
        Platform::Mutex m_mutex;
        mutable DateTime m_point;
        mutable dword m_counter;
        
    public:
        SyncPointAdder(const DateTime &startPoint, dword pointDiffMs) : 
            m_pointDiffMs(pointDiffMs), m_point(startPoint), m_counter(0)
        {
        }

        void AddPoint()
        {
            Platform::MutexLocker lock(m_mutex);
            m_point.AddTicksToDateTime(m_pointDiffMs);
            MsDateTime::SetSyncPoint(m_point);
            ++m_counter;
        }

        dword Counter()
        {
            dword res = 0;
            {
                Platform::MutexLocker lock(m_mutex);
                res = m_counter;
            }
            
            return res;
        }
    };
    
    // ---------------------------------------------------
    
    class DTSynchronizerThread :
        public Platform::Thread
    {
        SyncPointAdder &m_spAdder;
        dword m_setPointIntervalMs;
        bool m_break;
        
        void run() // override
        {
            while(!m_break)
            {
                m_spAdder.AddPoint();
                Platform::Thread::Sleep(m_setPointIntervalMs);
            }
        }

    public:
        DTSynchronizerThread(SyncPointAdder &spAdder, dword setPointIntervalMs) : 
            Platform::Thread("DTSynchronizerThread"),
            m_spAdder(spAdder), m_setPointIntervalMs(setPointIntervalMs), m_break(false)
        {
            start(Platform::Thread::LowPriority);
        }

        ~DTSynchronizerThread()
        {
            m_break = true;
            wait();
        }
    };

    // ---------------------------------------------------
    // Test multiple access to one MsDateTime
    void CaptureTestMT()
    {
        // start sync point
        DateTime syncPoint;
        syncPoint.date().Set(2009, 3, 9);
        syncPoint.time().Set(23, 59, 0);
        MsDateTime::SetSyncPoint(syncPoint);

        // init dt
        MsDateTime dt;
        dt.Capture();

        dword pointTimeDiffMs = 2 * 60 * 1000;
        SyncPointAdder adder(syncPoint, pointTimeDiffMs);
        {
            dword setPointIntervalMs = 100;
        
            // set point in diff thread
            DTSynchronizerThread t(adder, setPointIntervalMs);
            Platform::Thread::Sleep(setPointIntervalMs * 5);

            MsDateTime::SetSyncPoint(syncPoint); // set fake point

            Platform::Thread::Sleep(setPointIntervalMs * 5);
        }
        
        // check timestamp
        MsDateTime dtBefore(dt);
        dt.Capture();
        dword checkRangeMs = pointTimeDiffMs * adder.Counter() + 2 * 1000; // + 2 sec
        TUT_ASSERT(CheckDateTimeInRange(dt, dtBefore.date(), dtBefore.time(), checkRangeMs));
    }

    // ----------------------------------------------------

    void CheckSetPointTestMT()
    {
        // start sync point
        DateTime syncPoint;
        syncPoint.date().Set(2009, 3, 9);
        syncPoint.time().Set(0, 0, 0);
        
        SyncPointAdder adder(syncPoint, 50);
        {
            dword setPointIntervalMs = 20;
        
            DTSynchronizerThread t(adder, setPointIntervalMs);
            DTSynchronizerThread t2(adder, setPointIntervalMs);
            DTSynchronizerThread t3(adder, setPointIntervalMs);
            Platform::Thread::Sleep(setPointIntervalMs * 10);
        }
    }
    
    
} // namespace 

// ---------------------------------------------------

namespace UtilsTests
{
    void MsDateTimeCaptureTest()
    {
        InterfaceTest();

        SetSyncPointTest(CreateDT(Date(2001, 5, 6), Time(15, 14, 17, 458)));
        SetSyncPointTest(CreateDT(Date(2005, 1, 1), Time(11, 18, 19, 625)));
        SetSyncPointTest(CreateDT(Date(1985, 12, 31), Time(2, 8, 59, 111)));
        CheckSetPointTestMT();

        LongCounterTest();
        CaptureTest();
        CaptureTestMT();
    }
    
} // namespace UtilsTests
