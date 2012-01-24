#include "stdafx.h"
#include "RtpToTdmBuff.h"

namespace
{

    enum
    {
        CDisableOutput = false,
        CDisableRx     = false,

        CEnableStatsByDefault = true,
    };


} // namespace



namespace TdmMng
{
    void RtpToTdmBuff::IncreaseDepth()
    {
        dword newDepth = m_currDepth + (m_currDepth * m_prof.UpScalePercent) / 100;

        // write stat                            
        if (m_statActive) ++m_lastStat.IncDepthCount;

        if (newDepth <= m_prof.MaxBufferingDepth) m_currDepth = newDepth;

        // todo else ???
    }

    //-------------------------------------------------------------------------------------

    void RtpToTdmBuff::DropOldPackets()
    {
        for (int i = 0; i < m_prof.DropCount; ++i)
        {
            BidirBuffer *p = m_buffQueue.Pop();

            m_byteInQueue -= p->Size();

            delete p;
        }
    }

    //-------------------------------------------------------------------------------------

    void RtpToTdmBuff::PutData(BidirBuffer* pBuff)
    {
        ESS_ASSERT(pBuff != 0);
        ESS_ASSERT(pBuff->Size() > 0);

        if ((m_state == st_inactive) || (CDisableRx))
        {
            delete pBuff;
            return;
        }

        // overflow queue
        if (m_buffQueue.IsFull())
        {   
            // удалить N (m_prof.DropCount) старых пакетов
            DropOldPackets();

            // write stat                            
            if (m_statActive) ++m_lastStat.OverflowFifoCount;                
        }

        ESS_ASSERT( !m_buffQueue.IsFull() );

        m_buffQueue.Push(pBuff);

        int size = pBuff->Size();
        m_byteInQueue += size;
        if (m_statActive) m_lastStat.BufferingRtpPackSize.Add(pBuff->Size());            

        // state trigger
        if ( 
            ((m_state == st_buffering) || (m_state == st_firstBuffering)) &&
            (m_byteInQueue >= m_currDepth) 
            )
        {
            m_state = st_sending;
        }
    }

    //-------------------------------------------------------------------------------------

    // output to TDM
    bool RtpToTdmBuff::WriteToBlock( ITdmWriteWrapper &block )
    {
        if (m_state != st_sending) 
        {
            if (m_statActive && (st_buffering == m_state)) ++m_lastStat.SlipCount;
            return false;
        }

        if (CDisableOutput)  // debug mode, just clean fifo
        {
            int queueHalfSize = m_buffQueue.MaxSize() / 2;

            while(m_buffQueue.Size() > queueHalfSize)
            {
                m_byteInQueue -= m_buffQueue.Head()->Size();
                delete m_buffQueue.Pop();
            }

            return true;
        }

        const word requestedSize = block.BlockSize();

        if (requestedSize > m_byteInQueue) // проскальзывание
        {
            IncreaseDepth();
            // write stat                            
            if (m_statActive)
            {
                ++m_lastStat.GoesBufferingCount;
                ++m_lastStat.SlipCount;
            }

            m_state = st_buffering;
            return false;
        }

        int readCount = 0;        

        while(readCount < requestedSize)
        {
            Utils::BidirBuffer& buff = *m_buffQueue.Head();
            int size = buff.Size();
            if ( (readCount + size) <= requestedSize)
            {
                // read full BidirBuffer
                block.Write(m_timeSlot, readCount, buff, size);                 
                delete m_buffQueue.Pop();                    
            }
            else
            {
                // read segment of BidirBuffer
                size = requestedSize - readCount;
                block.Write(m_timeSlot, readCount, buff, size);                 
                buff.AddSpaceFront(-size);
            }

            ESS_ASSERT(size != 0);  // paranoic

            readCount += size;
            m_byteInQueue -= size;   
        }        

        // write stat                            
        if (m_statActive) 
        {
            ++m_lastStat.SuccessReadFromBuffCount;            
            m_lastStat.TotalOutByte += readCount;
        }

        return true;
    }

    //-------------------------------------------------------------------------------------

    RtpToTdmBuff::RtpToTdmBuff( const RtpRecvBufferProfile& prof, int timeSlot ) : 
		m_checkProfile(prof),
        m_state(st_inactive),         
        m_prof(prof),            
        m_buffQueue(prof.QueueDepth),            
        m_timeSlot(timeSlot)
    {
        m_statActive = CEnableStatsByDefault;
        
        Stop();
    }

    //-------------------------------------------------------------------------------------

    const RtpToTdmBufferStat& RtpToTdmBuff::getLastStat() const
    {
        return m_lastStat;
    }

    //-------------------------------------------------------------------------------------

    /*
    void RtpToTdmBuff::getAllTimeStat(RtpToTdmBufferStat& statOut) const
    {
        statOut.Clear();

        if (st_inactive)
        {
            statOut = m_allStat;
            return;
        }

        statOut.SummWith(m_lastStat);
    } */

    void RtpToTdmBuff::getAllTimeStat(RtpToTdmBufferStat& statOut) const
    {
        statOut = m_allStat;

        if (m_state == st_inactive) statOut.SummWith(m_lastStat);
    }


    //-------------------------------------------------------------------------------------

    void RtpToTdmBuff::Start()
    {
        ESS_ASSERT(m_state == st_inactive);
        
        m_state = st_firstBuffering;//st_buffering;
        m_lastStat.Clear();        
        m_currDepth = m_prof.MinBufferingDepth;
        m_byteInQueue = 0;            
    }

    //-------------------------------------------------------------------------------------

    void RtpToTdmBuff::Stop() /* after Stop() buffer is empty */
    {
        if (m_state != st_inactive) m_allStat.SummWith( m_lastStat );

        m_state = st_inactive;
        m_buffQueue.Clear();        
    }   

} // namespace TdmMng

