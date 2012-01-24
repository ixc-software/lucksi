#include "stdafx.h"
#include "RtpPacketStatistic.h"

#include "Utils/ErrorsSubsystem.h"

namespace iRtp
{

    void RtpPacketStatistic::EvalAvgSize( int newSize )
    {
        if (m_packetSizeIsConst)
        {
            if (newSize == m_packetSizeAvg)
                return;
            m_packetSizeIsConst = false;
        }

        m_packetSizeAvg = (m_packetSizeAvg * (m_packetCount -1) + newSize ) / m_packetCount;
    }

    //-------------------------------------------------------------------------------

    void RtpPacketStatistic::EvalAvgTs( dword newTs )
    {
        if (m_dTsIsConst)
        {
            if (newTs == m_dTsAvg)
                return; // nothing to do
            m_dTsIsConst = false;
        }

        m_dTsAvg = (m_dTsAvg * (m_packetCount -1) + newTs ) / m_packetCount;
    }

    //-------------------------------------------------------------------------------

    RtpPacketStatistic::RtpPacketStatistic() : m_lastHeader(false, 0, CEmptyPayloadType)
    {
        Reset();
    }

    //-------------------------------------------------------------------------------

    void RtpPacketStatistic::Reset()
    {
        m_packetCount = 0;
        m_dTsAvg = 0;
        m_dTsIsConst = true;
        m_packetSizeAvg = 0;
        m_packetSizeIsConst = true;

        //iRtp::RtpHeaderForUser(false, 0, iRtp::CEmptyPayloadType);
        m_lastSize = 0;
    }

    //-------------------------------------------------------------------------------

    void RtpPacketStatistic::Add( int size, RtpHeaderForUser header )
    {
        if (++m_packetCount == 1)// is first
        {
            m_lastHeader = header;
            m_lastSize = size;

            m_packetSizeAvg = size;
            return;
        }

        EvalAvgSize(size);

        if (++m_packetCount == 2)// second packet 
            m_dTsAvg = header.Timestamp - m_lastHeader.Timestamp;  
        else
            EvalAvgTs(header.Timestamp);     

        m_lastHeader = header;
        m_lastSize = size;
    }

    //-------------------------------------------------------------------------------

    int RtpPacketStatistic::getCount() const
    {
        return m_packetCount;
    }

    //-------------------------------------------------------------------------------

    iRtp::dword RtpPacketStatistic::getAvgTs() const
    {
        ESS_ASSERT(m_packetCount > 0);
        return m_dTsAvg;
    }

    //-------------------------------------------------------------------------------

    bool RtpPacketStatistic::TsIsConst() const
    {
        return m_dTsIsConst;
    }

    //-------------------------------------------------------------------------------

    int RtpPacketStatistic::getAvgSize() const
    {
        ESS_ASSERT(m_packetCount > 0);
        return m_packetSizeAvg;
    }

    //-------------------------------------------------------------------------------

    bool RtpPacketStatistic::SizeIsConst() const
    {
        return m_packetSizeIsConst;
    }
} // iRtp

