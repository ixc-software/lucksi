#ifndef SSRCSEQCOUNTER_H
#define SSRCSEQCOUNTER_H

#include "RtpParams.h"
#include "Platform/PlatformTypes.h"
#include "ssrcstatistic.h"

namespace iRtp
{
    using Platform::dword;

    class ISsrcCounterCallback : Utils::IBasicInterface
    {
    public:
        // ����������� � ������� ������������ �������������
        virtual void SynchroCompleteWith(dword ssrc) = 0;
    };

    class SsrcSeqCounter : boost::noncopyable
    { 
        enum {CBadSeqNum = -1};
        
        dword m_ssrc; // associated ssrc
        SeqCounterSettings m_settings;
        ISsrcCounterCallback& m_callback;

        word m_lastSeq;        /* highest seq. number seen */     
        int m_lastNonSeq;        /* last inconsequential seq number */ 
        dword m_probeCount;      /* seq. packets till source is valid */ 

        mutable SsrcStatistic m_stat;        
        
        // fields used only for create m_stat
        //word m_firstSeq;
        //word m_seqCycleCount; // ������� ������������ m_lastSeq

        word IncSeq(word seq);

        word EvalDelta(word seq);

        bool CountProbe(word seq);   

        bool ValidateSeq(word seq);

    public:

        SsrcSeqCounter(ISsrcCounterCallback& callback, dword ssrc, const SeqCounterSettings& settings)
            : m_ssrc(ssrc),
            m_settings(settings),
            m_callback(callback),
            m_lastSeq(0),        
            m_lastNonSeq(CBadSeqNum),
            m_probeCount(0)
            //m_firstSeq(0),
            //m_seqCycleCount(0)
        {}   

        bool ValidateSeqAndDoSizeStat(word seq, int packSizeForStatistic);

        dword getSSRC() const;

        const SsrcStatistic& getStat() const;
    }; 

    // ��� �������� �� �������� ������� �� ��������� ���������� �������������
    class SeqCounterContainer : public ISsrcCounterCallback, boost::noncopyable
    {
        // ��������������� �� ��������� �� ������������� ������ ����� ������
        // ������������ � ���������� SSRC ���� ��� ��������

        ISsrcCounterCallback& m_callback;
        SeqCounterSettings m_settings;
        boost::scoped_ptr<SsrcSeqCounter> m_counter;
        RtpStatistic m_stat;


    // ISsrcCounterCallback impl:
    private:
        void SynchroCompleteWith(dword ssrc);

    public:

        // ���� ssrc ��������������� - ����������� ���������, ���� ��� - ������ ������� �������������� ����� ssrc
        bool ValidateSeqAndDoSizeStatistic(word seq, dword ssrc, int packSizeForStat);

        SeqCounterContainer(ISsrcCounterCallback& callback, const RtpParams& params)
            : m_callback(callback),
            m_settings(params.SeqConterSettings)            
        {}


        // �������� ���������� �� ���� ������������ � ������� Ssrc + ���������� ����������
        void getAllTimeStats(RtpStatistic& statOut) const;
        
        // ���������� ������ �� ���������� ssrc
        void getLastSsrcStat(RtpStatistic& statOut) const
        {
            statOut.Clear();

            if ( m_counter.get() == 0) return;

            //statOut.SndPackSizeStat = m_sndPackSizeStat;
            statOut.SsrcRecvCount = 1;
            statOut.SsrcRegistredCount = 1;
            statOut.SsrcStat = m_counter->getStat();
        }


    };
} // namespace iRtp

#endif
