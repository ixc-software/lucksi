#include "stdafx.h"
#include "L2StatData.h"
#include "Utils/QtHelpers.h"
#include "Utils/ErrorsSubsystem.h"

namespace ISDN
{
    using Platform::dword;

    std::string L2StatData::getAsString() const
    {
        std::stringstream ss;
        ss << "\n\tTime at begin measurement: " << m_timeAtActivation.toString("hh:mm:ss")
			<< "\n\tStat collected period(sec): " << m_statAge
			<< "\n\tTx I-packet count: " << m_TxIcount
			<< "\n\tRx I-packet count: " << m_RxIcount
			<< "\n\tTx U-packet count: " << m_TxUcount
			<< "\n\tRx U-packet count: " << m_RxUcount
			<< "\n\tBad packet count: " << m_badCount
			<< "\n\tTotal count of Tx packet: " << m_TxAllPackCount
			<< "\n\tTotal count of Rx packet: " << m_RxAllPackCount
			<< "\n\tTotal time in active state(sec): " << m_totalSessionTime
			<< "\n\tLast activation at: " << m_timeAtStartSession.toString(Qt::SystemLocaleShortDate)
			<< "\n\tLast deactivation at: " << m_timeAtStopSession.toString(Qt::SystemLocaleShortDate);
        return ss.str();
    }

    //-----------------------------------------------------------------------------------------------------

    dword L2StatCollector::EvalPeriodSec(const QDateTime& current, const QDateTime& prev)
    {
        return prev.secsTo(current);
    }

    //-----------------------------------------------------------------------------------------------------

    void L2StatCollector::Init()
    {
        m_sessionStarted = false;
        m_data.Init();
    }

    //-----------------------------------------------------------------------------------------------------

    L2StatCollector::L2StatCollector(bool active, bool lapdActive)
        : m_active(false)           
    {
        Init();
        if (active) 
            ActivateStat(lapdActive);
    }        

    //-----------------------------------------------------------------------------------------------------

    void L2StatCollector::IPackTxInc()
    {
        if (!m_active) return;
        ++m_data.m_TxIcount;
    }

    //-----------------------------------------------------------------------------------------------------

    void L2StatCollector::UPackTxInc()
    {
        if (!m_active) return;
        ++m_data.m_TxUcount;
    }

    //-----------------------------------------------------------------------------------------------------

    void L2StatCollector::IPackRxInc()
    {
        if (!m_active) return;
        ++m_data.m_RxIcount;
    }

    //-----------------------------------------------------------------------------------------------------

    void L2StatCollector::UPackRxInc()
    {
        if (!m_active) return;
        ++m_data.m_RxUcount;
    }

    //-----------------------------------------------------------------------------------------------------

    void L2StatCollector::AllTxInc()
    {
        if (!m_active) return;
        ++m_data.m_TxAllPackCount;
    }

    //-----------------------------------------------------------------------------------------------------

    void L2StatCollector::AllRxInc()
    {
        if (!m_active) return;
        ++m_data.m_RxAllPackCount;
    }

    //-----------------------------------------------------------------------------------------------------

    void L2StatCollector::BadPackInc()
    {
        if (!m_active) return;
        ++m_data.m_badCount;
    }

    //-----------------------------------------------------------------------------------------------------

    void L2StatCollector::ActivateMoment()
    {
        if (!m_active) return;
        if (m_sessionStarted) return; // todo
        ESS_ASSERT(!m_sessionStarted);
        m_data.m_timeAtStartSession = QDateTime::currentDateTime();
        m_sessionStarted = true;
    }

    //-----------------------------------------------------------------------------------------------------

    void L2StatCollector::DeactivateMoment()
    {
        if (!m_active || !m_sessionStarted) return;        
		
		m_data.m_timeAtStopSession = QDateTime::currentDateTime();
        m_data.m_totalSessionTime += EvalPeriodSec(m_data.m_timeAtStopSession, m_data.m_timeAtStartSession);
        m_sessionStarted = false;
    }  

    //-----------------------------------------------------------------------------------------------------

    void L2StatCollector::ActivateStat(bool lapdActive)
    {
        ESS_ASSERT(!m_active);
        Clear();
        m_data.m_timeAtActivation = QDateTime::currentDateTime();
        m_active = true;
        if (lapdActive) ActivateMoment();
    }

    //-----------------------------------------------------------------------------------------------------

    void L2StatCollector::Clear()
    {
        Init();
    }

    //-----------------------------------------------------------------------------------------------------

    L2StatData L2StatCollector::getData() const
    {
        L2StatData ret(m_data);
        QDateTime now = QDateTime::currentDateTime();

        if (m_sessionStarted)
            ret.m_totalSessionTime += EvalPeriodSec(now, m_data.m_timeAtStartSession);
        if (m_active)
            ret.m_statAge = EvalPeriodSec(now, m_data.m_timeAtActivation);

        return ret;
    }
} // namespace ISDN

