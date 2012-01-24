#include "stdafx.h"
#include "TdmToTdmBuff.h"

namespace TdmMng
{
    void TdmToTdmBuff::OnConnect()
    {
        ESS_ASSERT(!m_connected);

        m_connected = true;
        m_lastStats.Clear();
        m_data.Clear();
    }

    //-------------------------------------------------------------------------------------

    void TdmToTdmBuff::OnData( DataPacket &data )
    {
        ESS_ASSERT(m_connected);

        if (!m_data.Empty()) m_lastStats.SlipsRecv++;

        m_data.DetachFrom( data );

        m_lastStats.ReadCount++;
    }

    //-------------------------------------------------------------------------------------

    void TdmToTdmBuff::OnDisconnect()
    {
        ESS_ASSERT(m_connected);

        m_allStats.SummWith(m_lastStats);
        m_connected = false;
    }

    //-------------------------------------------------------------------------------------

    bool TdmToTdmBuff::WriteToBlock( ITdmWriteWrapper &block )
    {
        ESS_ASSERT(m_connected);

        if (m_data.Empty())
        {
            m_lastStats.SlipsSend++;
            return false;
        }

        // move data from m_data to block
        m_data.WriteTo(m_inTimeSlot, block);

        m_data.Clear();
        return true;
    }

    //-------------------------------------------------------------------------------------

    TdmToTdmBuff::TdmToTdmBuff( ITdmManager &mng, Platform::byte inputTimeSlot ) : m_data(/*mng*/), m_inTimeSlot(inputTimeSlot)
    {
        m_connected = false;
    }

    //-------------------------------------------------------------------------------------

    const TdmToTdmBuffStat& TdmToTdmBuff::getLastStat() const
    {
        return m_lastStats;
    }

    //-------------------------------------------------------------------------------------

    /*
    void TdmToTdmBuff::getAllTimeStat( TdmToTdmBuffStat& statOut ) const
    {
        statOut.Clear();

        if (m_connected)
        {
            statOut = m_allStats;
            return;
        }

        statOut.SummWith(m_lastStats);
    } */

    void TdmToTdmBuff::getAllTimeStat( TdmToTdmBuffStat& statOut ) const
    {
        statOut = m_allStats;

        if (m_connected) statOut.SummWith(m_lastStats);
    }

} // namespace TdmMng

