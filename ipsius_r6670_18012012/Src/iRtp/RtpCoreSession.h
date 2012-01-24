#ifndef RTPSESSION_BF_H
#define RTPSESSION_BF_H

#include "RtpSocketInterfaces.h"
#include "IRtpCoreToUser.h"

#include "RtpParams.h"

#include "IRtpInfra.h"

#include "SsrcSeqCounter.h"
#include "iLog/LogWrapper.h"

namespace iRtp
{

    using Utils::BidirBuffer;

    class RtpCoreSession : boost::noncopyable,
        public ISocketToRtp,  
        public ISsrcCounterCallback
    {
        enum 
        {
            CPreallocPack = 256, 
            CPacketSizeof = 50
        };

        struct RtpHeader;
        
		const RtpParams m_prof;
		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;
		iLogW::LogRecordTag m_errorTag;
        RtpPayload m_txPayload;
        SeqCounterContainer m_seqCounters;        
        IRtpInfra& m_infra;
        IRtpCoreToUser& m_user;

        dword m_ownerSsrc; // идентификатор источника (если реализовывать множество источников set<dword>)
        word m_lastSeq;

        Utils::StatElementForInt m_txLastStat;
        Utils::StatElementForInt m_txAllStat;
        int m_socketErrLastStat;
        int m_socketErrAllStat;

        SocketPair m_sockets;
        bool m_destIsSet; // признак того что удаленный адрес назначен

        bool ValidateSocketId(SocketId id);
        
        //-----------------------------------------
        // обработка пакетов

        // Extract rtp-header, hide non-user data in pBuff. Return true if parsing complete
        bool ParseAndConvertData(BidirBuffer* pBuff, RtpHeader& rezult);

        void AttachRtpFieldToBuffer(bool marker, dword ts, BidirBuffer* pUserData);

        word getNextSeq(/*dword ssrc*/);

        void ClearDstAddr(/*todo HostInf dstAddr*/)
        {
            m_destIsSet = false;
            m_txAllStat += m_txLastStat;
            m_socketErrAllStat += m_socketErrLastStat;

            // clear last
            m_txLastStat.Clear();
            m_socketErrLastStat = 0;
        }

    // ISsrcCounterCallback
    private:
        void SynchroCompleteWith(dword ssrc);

    // ISocketToRtp impl
    private:
        
        void RxData(SocketId id, BidirBuffer* pData, const HostInf& srcAddr);
        void ErrorInd(SocketId id, RtpError err);
    
    // User interface:
    public:
        
        //! TxData принимает pBuff согласно идиоме владения
        //(те после вызова TxData pBuff может стать недействительным)
        void TxData(bool marker, dword timestamp, BidirBuffer* pData);        
        void TxEvent(dword timestamp, RtpEvent ev);

        void setPayload(RtpPayload payloadType);
        void setDestAddr(HostInf dstAddr);

        const HostInf& getLocalRtpAddr() const
        {
            return m_sockets.RtpSock->getLocalAddr();
        }
        const HostInf& getLocalRtcpAddr() const
        {
            return m_sockets.RtcpSock->getLocalAddr();
        }

        // Blackfin only
        void SocketPoll(bool dropPacket, Utils::StatElementForInt &sockStat)
        {
            m_sockets.Poll(dropPacket, sockStat);
        }

        // debug
        void RtpSockDirectWrite(BidirBuffer* pData);

    public:
        
        RtpCoreSession(const RtpParams& prof, 
			iLogW::ILogSessionCreator &logCreator,
            IRtpInfra& infra, 
            IRtpCoreToUser& user, 
            const HostInf &dstAddr = HostInf());  

		~RtpCoreSession();

        void getAllTimeStats(RtpStatistic& statOut) const
        {
            statOut.Clear();
            m_seqCounters.getAllTimeStats(statOut);                        
            // add sender Statistic
            statOut.SendPackSizeStat = m_txAllStat;
            statOut.SocketErrorCounter = m_socketErrAllStat;
            if (m_destIsSet) 
            {
                statOut.SendPackSizeStat += m_txLastStat;
                statOut.SocketErrorCounter += m_socketErrLastStat;
            }
        }

        void getLastSsrcStats(RtpStatistic &statOut) const
        {
            statOut.Clear();
            m_seqCounters.getLastSsrcStat(statOut);
            statOut.SendPackSizeStat = m_txLastStat;
            statOut.SocketErrorCounter = m_socketErrLastStat;
        }


    };    
    

} // namespace iRtp

#endif
