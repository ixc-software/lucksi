#include "stdafx.h"
#include "SsrcSeqCounter.h"
#include "RtpConstants.h"

namespace iRtp
{
    word SsrcSeqCounter::IncSeq( word seq )
    {
        /*if (seq == CRtpSequenceMod)
        {
            //++m_seqCycleCount;
            return 0;
        }
        else*/
        return seq + 1;

        //return (seq + 1) & (m_seqMod - 1); , ������ ���� m_seqMod == 2^n
    }

    //---------------------------------------------------------------------

    iRtp::word SsrcSeqCounter::EvalDelta( word seq )
    {
        return seq - m_lastSeq;        
    }

    //---------------------------------------------------------------------

    bool SsrcSeqCounter::CountProbe( word seq )
    {
        if (m_probeCount == 0)// ������ �������
        {
            //m_firstSeq = seq; // only for statistic
            m_lastSeq = seq;
            ++m_probeCount;
            return false;
        }


        word nextSeq = IncSeq(m_lastSeq);
        if (seq == nextSeq)       
        {
            m_lastSeq = seq;
            return ++m_probeCount == m_settings.MinSequential;
        }

        m_probeCount = 1; // ����������� ������� � 1
        m_lastSeq = seq;        
        return false;
    }

    //---------------------------------------------------------------------

    bool SsrcSeqCounter::ValidateSeq(word seq)
    {
        
        if (m_probeCount < m_settings.MinSequential) // �������� ��� �� ����� ����������� ��� ����������� ���-�� �������
        {
            bool synchroComplete = CountProbe(seq);
            if (synchroComplete) m_callback.SynchroCompleteWith(m_ssrc);            
            return synchroComplete;
        }

        if (seq == IncSeq(m_lastSeq))
        {
            m_lastSeq = seq;
            return true;
        }

        ++m_stat.SeqFailureCounter;

        word udelta = EvalDelta(seq); // evaluate difference
        if (udelta < m_settings.MaxDropOut) // ������� � �������� ����������� ����� ���������
        {
            m_lastSeq = seq;
            return true;
        }        

        if (udelta <= CRtpSequenceMod - m_settings.MaxMisOrder) //���� ��������� � �������� �����
        {
            word nextSeq = IncSeq(m_lastNonSeq);
            if (seq == nextSeq) // ������ ����� ����� ���� �������������� ��� ���������������
            {
                ++m_stat.ReSynchroCounter; // ������� ���������������
                m_lastNonSeq = CBadSeqNum;
                m_lastSeq = seq;
                return true;
            }
            else
            {
                m_lastNonSeq = seq;                
                return false;
            }
        }
        
        return false; //���� ��������� ��� �����
    }

    bool SsrcSeqCounter::ValidateSeqAndDoSizeStat( word seq, int packSizeForStatistic )
    {
        ++m_stat.RecvPacketCount;
        bool result = ValidateSeq(seq);
        if (!result) ++m_stat.DropCount;
        else m_stat.RecvPackSizeStat.Add(packSizeForStatistic);
        return result;
    }
    

    //---------------------------------------------------------------------

    iRtp::dword SsrcSeqCounter::getSSRC() const
    {
        return m_ssrc;
    }
    
    //---------------------------------------------------------------------

    const iRtp::SsrcStatistic& SsrcSeqCounter::getStat() const
    {
        //if (m_probeCount != 0)
        //{
        //    //����� ��������� = ����� ��������� - ������� ��������. �������� ������ �� �������� m_settings.MaxMisOrder
        //    Platform::word estimated = m_lastSeq - m_firstSeq + 1; // todo ������ m_seqCycleCount           
        //    m_stat.LostCount = (estimated > m_stat.RecvPacketCount) ? estimated - m_stat.RecvPacketCount : 0;
        //}

        return m_stat;
    }   

    //---------------------------------------------------------------------

    // ����� SsrcSeqCounter �� SSRC, ������� ����� ���� �� ������, ������������ ��� ������
    //bool SeqCounterContainer::ValidateSeq( word seq, dword ssrc )
    //{
    //    if (m_counter.get() == 0)
    //        m_counter.reset(new SsrcSeqCounter(ssrc, m_settings)); // todo use pool
    //    ESS_ASSERT(ssrc == m_counter->getSSRC()); // ������-�� return false;
    //    return m_counter->ValidateSeqNum(seq);
    //}

    bool SeqCounterContainer::ValidateSeqAndDoSizeStatistic( word seq, dword ssrc, int packSizeForStat)
    {
        // ����� �������� ���������� ��������� ��������� ����� ����������
        if (m_counter == 0 || ssrc != m_counter->getSSRC())
        {
            if (m_counter != 0) m_stat.SsrcStat.SummWith( m_counter->getStat() );            
            
            m_counter.reset(new SsrcSeqCounter(*this, ssrc, m_settings)); 
            
            ++m_stat.SsrcRecvCount;
            
        }
        
        return m_counter->ValidateSeqAndDoSizeStat(seq, packSizeForStat);
    }

    // �������� ���������� �� ���� ������������ � ������� Ssrc + ���������� ����������
    void SeqCounterContainer::getAllTimeStats(RtpStatistic& statOut) const
    {                
        if (m_counter == 0) 
        {            
            statOut = m_stat;
            return;
        }

        statOut = m_stat;
        statOut.SsrcStat.SummWith( m_counter->getStat() ); 
        return;
    }

    //-------------------------------------------------------------------------------------

    

    //-------------------------------------------------------------------------------------

    void SeqCounterContainer::SynchroCompleteWith( dword ssrc )
    {
        // todo �������� ������ ������ ��� ����������. �������� �� ������ �������.
        ++m_stat.SsrcRegistredCount;
        m_callback.SynchroCompleteWith(ssrc);
    }
   
} // namespace iRtp

