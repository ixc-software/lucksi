#include "stdafx.h"
#include "SendToRtp.h"
#include "BoolALawToRtpPayload.h"
#include "Utils/HostInf.h"

namespace 
{
    const int CRtpPackSize = 160; // размер пакетов отправляемых в RtpCore

    const bool CDisableRtpSend = false;  // debug

    const bool CUseDirectUnbuffSend = true;

} // namespace 

namespace TdmMng
{
    using Utils::HostInf;


    // ------------------------------------------------------------

    void SendToRtp::PackToRtp(Utils::BidirBuffer *p)
    {
        ESS_ASSERT(p != 0);

        if (CDisableRtpSend)  // debug path
        {
            delete p;
            return;
        }

        bool marker = false; // на данный момент нет требования к маркеру потому ставим любой
        int size = p->Size();

        m_rtp.TxData(marker, m_sampleCounter, p);

        m_sampleCounter += size;
    }

    // ------------------------------------------------------------

    void SendToRtp::SendBuffered(DataPacket &pack, bool canDetach)
    {
        const int CReadNothing = -1;
        int readOffset = CReadNothing; // смещение с которого продолжаем чтение если необходимо

        {
            int shortage = CRtpPackSize - m_pBuff->Size();
            int readSize = (shortage >= pack.Buffer().Size()) ? pack.Buffer().Size() : shortage;
            m_pBuff->PushBack(pack.Buffer().Front(), readSize);
            if (readSize < pack.Buffer().Size()) readOffset = readSize;
        }

        if (m_pBuff->Size() >= CRtpPackSize)
        {
            PackToRtp(m_pBuff);
            m_pBuff = m_mng.getRtpInfra().CreateBuff();
        }

        if (readOffset != CReadNothing)
        {
            ESS_ASSERT(m_pBuff->Size() == 0);
            m_pBuff->PushBack(&pack.Buffer().At(readOffset), pack.Buffer().Size() - readOffset);
        }
    }

    // ------------------------------------------------------------

    void SendToRtp::SendDirect(DataPacket &pack, bool canDetach)
    {
        ESS_ASSERT(canDetach);

        Utils::BidirBuffer *p = pack.DetachBuff();

        PackToRtp(p);        
    }


    // ------------------------------------------------------------

    void SendToRtp::Send( DataPacket &pack, bool canDetach )
    {
        ESS_ASSERT(pack.Buffer().Size() <= CRtpPackSize);        
        ESS_ASSERT(pack.Buffer().Size() != 0);

        if (CUseDirectUnbuffSend) SendDirect(pack, canDetach);
                            else  SendBuffered(pack, canDetach);

    }

    //-------------------------------------------------------------------------------------

    bool SendToRtp::Equal( const std::string &ip, int port ) const
    {
        return m_id.Equal(ip, port);
    }

    //-------------------------------------------------------------------------------------

    SendToRtp::SendToRtp( ITdmManager& mng, const std::string &ip, int port, bool useAlaw, iRtp::RtpCoreSession &rtp ) 
        : m_id(ip, port), 
        m_mng(mng),
        m_rtp(rtp),
        m_sampleCounter(0),
        m_pBuff( m_mng.getRtpInfra().CreateBuff() )
    {
        Utils::HostInf dstAddr(ip, port);        

        m_rtp.setDestAddr(dstAddr);
        m_rtp.setPayload( BoolALawToRtpPayload(useAlaw) );
    }

    //-------------------------------------------------------------------------------------

    SendToRtp::~SendToRtp()
    {
        delete m_pBuff;
    }

    //-------------------------------------------------------------------------------------

    int SendToRtp::RtpPackSize()
    {
        return CRtpPackSize;
    }
} // TdmMng

