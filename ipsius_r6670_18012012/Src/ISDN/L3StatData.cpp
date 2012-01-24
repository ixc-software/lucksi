#include "stdafx.h"
#include "L3StatData.h"
#include "Utils/QtHelpers.h"
#include "Utils/ErrorsSubsystem.h"

namespace ISDN
{
    std::string L3StatData::getAsString() const
    {
        std::stringstream ss;
        ss << "\n\tTime at begin measurement: " << m_startStat.toString("hh:mm:ss")
			<< "\n\tStat collected period(sec): " << m_statAge
			<< "\n\tEnter in active state count: " << m_activateDssCount
			<< "\n\tTotal time in active state(sec): " << m_totalActiveTime
			<< "\n\tTime at last activation: " << m_lastDssActivation.toString("hh:mm:ss")
			<< "\n\tTx packet count: " << m_txCount
			<< "\n\tRx packet count: " << m_rxCount
			<< "\n\tRx bad packet count: " << m_badRxCount
			<< "\n\tCall created count: " << m_callCount
			<< "\n\tTotal call connected count: " << m_talkCount
			<< "\n\tCurrent call connected count: " << m_currentTalkCount
			<< "\n\tTotal talk time(sec): " << m_totalTalkTime / 1000;        

        return ss.str();
    }

    //-----------------------------------------------------------------------------------------------------

    void L3StatCollector::Init()
    {
        m_statActive = false;
        m_dssInActive = false;
        m_data.Init();
    }

    //-----------------------------------------------------------------------------------------------------
    
    void L3StatCollector::ChangeTalkCount(int count)
    {                        
        int evalPeriod = QTime::currentTime().msecsTo(m_data.m_lastChangeTalkCount.time());            
        m_data.m_totalTalkTime -= evalPeriod * m_data.m_currentTalkCount;
        m_data.m_currentTalkCount += count;
        m_data.m_lastChangeTalkCount = QDateTime::currentDateTime();

        if (count > 0)
            m_data.m_talkCount += count;
    }       

    //-----------------------------------------------------------------------------------------------------

    int L3StatCollector::EvalPeriodSec(const QDateTime& from, const QDateTime& to)
    {
        return from.secsTo(to);
    }    

    //-----------------------------------------------------------------------------------------------------

    L3StatCollector::L3StatCollector(bool statActive, bool dssActive)
        : m_statActive(false)
    {
        if (statActive)
            ActivateStat(dssActive);
    }

    //-----------------------------------------------------------------------------------------------------

    void L3StatCollector::ActivateStat(bool dssActive)
    {
        ESS_ASSERT(!m_statActive);
        Clear();
        m_data.m_startStat = QDateTime::currentDateTime();
        m_statActive = true;
        if (dssActive)
            DssActivated();
    }

    //-----------------------------------------------------------------------------------------------------

    void L3StatCollector::DeactivateStat()
    {
        ESS_ASSERT(m_statActive);
        m_data.m_statAge += EvalPeriodSec(m_data.m_startStat, QDateTime::currentDateTime());
        m_statActive = false;
    }

    //-----------------------------------------------------------------------------------------------------

    void L3StatCollector::Clear()
    {
        Init();
    }

    //-----------------------------------------------------------------------------------------------------

    void L3StatCollector::TxInc()
    {
        if (m_statActive) ++m_data.m_txCount;
    }

    //-----------------------------------------------------------------------------------------------------

    void L3StatCollector::RxInc()
    {
        if (m_statActive) ++m_data.m_rxCount;
    }

    //-----------------------------------------------------------------------------------------------------

    void L3StatCollector::BadPackInc()
    {
        if (m_statActive) ++m_data.m_badRxCount;
    }

    //-----------------------------------------------------------------------------------------------------

    void L3StatCollector::CallInc()
    {
        if (m_statActive) ++m_data.m_callCount;
    }

    //-----------------------------------------------------------------------------------------------------

    void L3StatCollector::TalkInc()
    {
        if (m_statActive) ChangeTalkCount(1);
    }

    //-----------------------------------------------------------------------------------------------------

    void L3StatCollector::TalkDec()
    {
        if (m_statActive) ChangeTalkCount(-1);
    }

    //-----------------------------------------------------------------------------------------------------

    void L3StatCollector::DssActivated()
    {                        
        if (m_statActive)
        {
            ESS_ASSERT(!m_dssInActive);
            m_data.m_lastDssActivation = QDateTime::currentDateTime();
            m_dssInActive = true;
            ++m_data.m_activateDssCount;
        }
    }

    //-----------------------------------------------------------------------------------------------------

    void L3StatCollector::DssDeactivated()
    {
        if (m_statActive)
        {
            ESS_ASSERT(m_dssInActive);
            QDateTime now = QDateTime::currentDateTime();
            m_data.m_totalActiveTime += EvalPeriodSec(m_data.m_lastDssActivation, now);
            m_data.m_lastDssActivation = now;                
            m_dssInActive = false;
        }
    }

    //-----------------------------------------------------------------------------------------------------

    L3StatData L3StatCollector::getData() const
    {
        L3StatData ret(m_data);
        QDateTime now = QDateTime::currentDateTime();

        if (m_statActive)
            ret.m_statAge += EvalPeriodSec(m_data.m_startStat, now);
        if (m_dssInActive)
            ret.m_totalActiveTime += EvalPeriodSec(m_data.m_lastDssActivation, now);

        return ret;
    }

} // namespace ISDN

