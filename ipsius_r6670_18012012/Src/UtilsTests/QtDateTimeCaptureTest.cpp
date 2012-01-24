
#include "stdafx.h"
#include "UtilsTests/DateTimeCaptureTests.h"
#include "Utils/QtDateTime.h"
#include "Platform/PlatformThread.h"


namespace UtilsTests
{
    using namespace Utils;

    /*
    QtDateTime()
    void Clear()
    bool IsNull() const
    void Capture()
    const Date& date() const
    const Time& time() const
    const DateTime& DateAndTime() const
    int Compare(const QtDateTime &other) const
    */
    void QtDateTimeCaptureTest()
    {
        QtDateTime qdt;
        QtDateTime qdt2;
        TUT_ASSERT(qdt.IsNull());
        TUT_ASSERT(qdt2.IsNull());
        TUT_ASSERT(qdt.Compare(qdt2) == 0);
        
        qdt.Capture();
        QDateTime checkTime = QDateTime::currentDateTime();
        
        TUT_ASSERT(!qdt.IsNull());
        TUT_ASSERT(!qdt.date().IsNull());
        TUT_ASSERT(!qdt.time().IsNull());
        TUT_ASSERT(!qdt.DateAndTime().IsNull());
        DateTime dt = qdt.DateAndTime();
        TUT_ASSERT(dt.date() == qdt.date());
        TUT_ASSERT(dt.time() == qdt.time());
        
        TUT_ASSERT(qdt.date().Day() == checkTime.date().day());
        TUT_ASSERT(qdt.date().Month() == checkTime.date().month());
        TUT_ASSERT(qdt.date().Year() == checkTime.date().year());
        TUT_ASSERT(qdt.time().Hour() == checkTime.time().hour());
        TUT_ASSERT(qdt.time().Min() == checkTime.time().minute());

        Platform::Thread::Sleep(50);
        qdt2.Capture();
        TUT_ASSERT(qdt.Compare(qdt2) < 0);

        qdt.Clear();
        TUT_ASSERT(qdt.IsNull());
        TUT_ASSERT(qdt.Compare(qdt2) < 0);
        TUT_ASSERT(qdt2.Compare(qdt) > 0);

        qdt.Capture();
        qdt.Capture();
        TUT_ASSERT(!qdt.IsNull());
    }
    
} // namespace UtilsTests
