#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/MsDateTimeHelpers.h"
#include "Utils/TimerTicks.h"
#include "Utils/DelayInit.h"
#include "MsDateTime.h"

using namespace Utils;
using namespace Utils::Detail;

typedef UtilsDetails::ThreadStrategy ThreadStrategy;

// -------------------------------------------------

namespace
{
    // определяет максимальный период, в течении которого можно НЕ вызывать функцию SetLongCounter
    // проверка сделана для того, чтобы система поставила опрос тиков на регулярную основу и не "прозевала" 
    // заворот 32-х разрядного счетчика тиков
    const int   CMaxCallPeriod = 60 * 60 * 1000;

    int         GTicksHiPart = -1;  // расширения разрядности системного счетчика тиков
    dword       GLastPollTime;      // время последнего вызова функции SetLongCounter

    void SetLongCounter(LongCounter &lc)
    {
        dword curr = Platform::GetSystemTickCount();

        if (GTicksHiPart < 0)   // first call
        {
            GTicksHiPart = 0;
        }
        else
        {
            if (CMaxCallPeriod > 0)
            {
                if (curr - GLastPollTime > CMaxCallPeriod) ESS_HALT("Time max period!");
            }

            // overrun detection
            if (curr < GLastPollTime) GTicksHiPart++;
        }

        // fill result
        lc.lo = curr;
        lc.hi = GTicksHiPart;

        // set last time
        GLastPollTime = curr;
    }

}

// -------------------------------------------------

namespace Utils
{
    namespace
    {
        /*
             Максимальное число точек синхронизации в GSyncPoints
             Фактически GSyncPoints это массив известного размера, resize
             вектора не должен происходить, т.к. на его элементы храняться
             указатели.
             Использование индексов вместо указателей приводит к тому, что
             мютекс для работы будет требовать функция ResolveTimestamp
        */
        const int CMaxSyncPoint = 128;

        ThreadStrategy::Mutex   *PGMutex;
        Utils::DelayInit        MutexInit(PGMutex);

        std::vector<SyncPoint>  GSyncPoints;

        void SetSyncPointWithoutMutex(const DateTime &datetime)
        {
            SyncPoint point;
            point.datetime = datetime;
            SetLongCounter(point.counter);

            int size = GSyncPoints.size();
            if (size == 0) GSyncPoints.reserve(CMaxSyncPoint);
                      else ESS_ASSERT(size < CMaxSyncPoint);

            GSyncPoints.push_back(point);
        }


    }  // namespace

    // ------------------------------------------------------------

    void MsDateTime::SetSyncPoint(const DateTime &datetime)
    {
        ESS_ASSERT(PGMutex != 0);
        ThreadStrategy::Locker lock(*PGMutex);
        SetSyncPointWithoutMutex(datetime);
    }

    // ------------------------------------------------------------

    void MsDateTime::GetTimestamp(Timestamp &ts)
    {
        ESS_ASSERT(PGMutex != 0);
        ThreadStrategy::Locker lock(*PGMutex);

        // if first run -- add default point
        if (GSyncPoints.size() == 0)
        {
            DateTime dt;
            UtilsDetails::DefaultValue::Set(dt);
            SetSyncPointWithoutMutex(dt);
        }

        // set long counter
        SetLongCounter(ts.counter);

        // set sync point
        int lastIndx = GSyncPoints.size() - 1;
        ts.pSyncPoint = &(GSyncPoints[lastIndx]);
    }

    // ------------------------------------------------------------

    void MsDateTime::ResolveTimestamp(const Timestamp &ts, DateTime &dt)
    {
        const dword CMsInDay = 1000 * 60 * 60 * 24;

        // 1. calc valTs as difference form SyncPoint
        ddword valTs;
        ts.counter.SetToDDword(valTs);

        SyncPoint *pSp = ts.pSyncPoint;
        ESS_ASSERT(pSp != 0);

        {
            ddword valSP;
            pSp->counter.SetToDDword(valSP);
            ESS_ASSERT(valTs >= valSP);
            valTs = valTs - valSP;
        }

        // 2. add to dt difference from valTs
        dt = pSp->datetime;

        dword dayPart  = valTs / CMsInDay;
        dword timePart = valTs % CMsInDay;
       
        int daysAdd = dt.time().AddTicksToTime(timePart);
        dt.date().AddDaysToDate(dayPart + daysAdd);
    }

    // ------------------------------------------------------------

    int MsDateTime::Compare(const MsDateTime &other) const
    {
        // return DateAndTime().Compare( other.DateAndTime() );

        if (IsNull()) return (other.IsNull())? 0 : -1;
        if (other.IsNull()) return (IsNull())? 0 : 1;
        
        return DateAndTime().Compare( other.DateAndTime() );
    }


}  // namespace Utils

