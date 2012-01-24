#include "stdafx.h"

//#include "iVDK/CriticalRegion.h"
//#include "iVDK/VdkUtils.h"

#include "RtpCoreSocket.h"

namespace
{
	
    enum
    {
        CDropAllPackets = false,

        CDisableSockSend = false,
    };


	/*

    int GEnterReadTime = 0;
    int GMaxReadTime = 0;

    void SockReadEnter()
    {
        int currTime = iVDK::GetSystemTickCount();

        iVDK::CriticalRegion::Enter();

        GEnterReadTime = currTime;

        iVDK::CriticalRegion::Leave();
    }

    void SockReadLeave()
    {
        int currTime = iVDK::GetSystemTickCount();

        iVDK::CriticalRegion::Enter();

        int diff = currTime - GEnterReadTime;
        GEnterReadTime = 0;

        if (diff > GMaxReadTime) GMaxReadTime = diff;
        
        iVDK::CriticalRegion::Leave();
    } */

}  // namespace


namespace iRtp
{         

	/*
    void RtpCoreSocket::CheckSocketState()
    {
        int currTime = iVDK::GetSystemTickCount();

        int maxSockReadTime = GMaxReadTime;
        int currReadTime = GEnterReadTime;

        int diff = currReadTime - currTime;

        bool sockInRead = (currReadTime > 0);

        int dummy = 0;  // for break        
    } */

    // -------------------------------------------------------------------------------------

    bool RtpCoreSocket::ReadToRecvBuff(Utils::HostInf &resAddr)
    {
        if (m_pRecvBuff == 0)
        {
            // create fresh buffer
            m_pRecvBuff = m_infra.CreateBuff(0);
            m_pRecvBuff->SetDataOffset(0, m_pRecvBuff->getBlockSize()); // set max available size
        }

        // SockReadEnter();

        bool packetCut = false;
        int recvSize = 
            m_lwSocket.Recv(m_pRecvBuff->Front(), m_pRecvBuff->Size(), resAddr, &packetCut);

        // SockReadLeave();

        // packet cut
        if (packetCut)
        {
            RtpError err(RtpError::SocketIncomingPackSplit, 
                         "Received datagram greater then given buffer.");
            m_rtp.ErrorInd(this, err);
        }

        // нет данных
        if (recvSize <= 0) return false;

        // корректируем размер буфера под принятые данные 
        m_pRecvBuff->SetDataOffset(0, recvSize); 
        
        return true;
    }

    // -------------------------------------------------------------------------------------
              
    void RtpCoreSocket::SocketPoll(bool dropPackets, Utils::StatElementForInt &sockStat)
    {
        Utils::HostInf addr;

        if (CDropAllPackets) dropPackets = true;
        
        // debug
        // if (dropPackets) return;  // помогает

        // read all data
        while( ReadToRecvBuff(addr) )
        {
        	
            if (m_ownPort == 2000)  // debug
            {
                int dummy = 1;
            }
        	        	
            // report to session
            if (!dropPackets)
            {
                sockStat.Add( m_pRecvBuff->Size() );
                m_rtp.RxData(this, m_pRecvBuff, addr);
                m_pRecvBuff = 0; // lost ownership
            }
            else
            {
                // clean buffer
                m_pRecvBuff->SetDataOffset(0, m_pRecvBuff->getBlockSize()); 
            }
                        
        }

    }      

    //-------------------------------------------------------------------------------------

    void RtpCoreSocket::TxData(BidirBuffer* pData, const HostInf& dstAddr)
    {                        
        /*
        if (dstAddr != m_dst)
        {
            m_LwDstCached.set( dstAddr.getAddress().toString() );
        } */


        if (!CDisableSockSend)
        {
            bool ok = m_lwSocket.SendTo(dstAddr, pData->Front(), pData->Size(), 
                &m_sendErrDesc);

            if (!ok)
            {
                m_rtp.ErrorInd(this, RtpError(RtpError::SocketSendFail, m_sendErrDesc) );
            }
        }
               
        delete pData;
    }

    //-------------------------------------------------------------------------------------

    void RtpCoreSocket::TxData(BidirBuffer* pData)
    {
        ESS_ASSERT(m_dstIsSet);
        TxData(pData, m_dst);
    }

    //-------------------------------------------------------------------------------------

    void RtpCoreSocket::SetDstAddress(const HostInf& dstAddr)
    {
        m_dstIsSet = true;
        m_dst = dstAddr;
    }

    //-------------------------------------------------------------------------------------     

    RtpCoreSocket::RtpCoreSocket( ISocketToRtp& rtp, IRtpInfra& infra, const HostInf& localAddr ) 
        : m_lwSocket( localAddr.Port() ),
        m_rtp(rtp),
        m_infra(infra),
        m_pRecvBuff(0),
        m_dstIsSet(false),
        m_local(localAddr)
    {
        m_ownPort = localAddr.Port();
    }       
        
}// namespace iRtp
