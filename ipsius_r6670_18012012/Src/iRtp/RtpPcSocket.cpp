#include "stdafx.h"
#include "RtpPcSocket.h"
#include "IRtpInfra.h"
#include "RtpError.h"
#include "iNet/SocketError.h"
#include "Utils/GlobalMutex.h"


namespace iRtp
{
    //---------------------------------------------------------------------

    const HostInf& RtpPcSocket::getLocalAddr() const
    {            
        return m_socket.LocalInf();
    }

    //---------------------------------------------------------------------

    void RtpPcSocket::TxData(BidirBuffer* pBuff, const HostInf& dstAddr)
    {
        // deep copy
        void* pData = pBuff->Front();
        QByteArray qData(static_cast<const char*>(pData), pBuff->Size());

        delete pBuff;

        boost::shared_ptr<iNet::SocketData> data( new iNet::SocketData(dstAddr, qData) );

        m_socket.SendData(data);
    }

    //---------------------------------------------------------------------

    /*
    bool RtpPcSocket::BindToPort(int port)
    {
        return m_socket.Bind(port);
    } */

    //---------------------------------------------------------------------

    void RtpPcSocket::SetDstAddress(const HostInf& dstAddr)
    {
        m_dstAddr = dstAddr;
        m_dstAddrInited = true;
    }

    //---------------------------------------------------------------------

    void RtpPcSocket::TxData(BidirBuffer* pData)
    {
        ESS_ASSERT(m_dstAddrInited);
        TxData(pData, m_dstAddr);
    }

    //---------------------------------------------------------------------

    void RtpPcSocket::ReceiveData(boost::shared_ptr<iNet::SocketData> data)
    {        
        BidirBuffer* pBuff = m_infra.CreateBuff(0);
        QByteArray qData = data->getData();
        pBuff->PushBack(qData.data(), qData.size());
		
        m_rtp.RxData(this, pBuff, data->getHostinf());
    }

    //---------------------------------------------------------------------

    void RtpPcSocket::SocketErrorOccur(boost::shared_ptr<iNet::SocketError> error)
    {
        RtpError err(RtpError::SocketPcError, error->getErrorString().toStdString());
        m_rtp.ErrorInd(this, err);
    }

    //---------------------------------------------------------------------


    bool RtpPcSocket::Bind(const HostInf &address)
    {
        return m_socket.Bind(address);
    }

    //---------------------------------------------------------------------

    RtpPcSocket::RtpPcSocket(iCore::MsgThread &thread, ISocketToRtp& rtp, IRtpInfra& infra)
        : m_rtp(rtp),
        m_socket(thread, this),
        m_dstAddrInited(false),
        m_infra(infra)
    {}
} // namespace iRtp

