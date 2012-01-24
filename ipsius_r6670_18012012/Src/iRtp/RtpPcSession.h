#ifndef RTPSESSION_PC_H
#define RTPSESSION_PC_H

#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"
#include "iLog/LogWrapper.h"
#include "RtpCoreSession.h"

#include "RtpSocketInterfaces.h"
#include "IRtpPCToUser.h"
#include "RtpPcInfra.h"

namespace iRtp
{
    class RtpPcSession : boost::noncopyable,
        public iCore::MsgObject,                        
        private IRtpCoreToUser // ��������� ����� � �������������� �������        
    {        
        typedef RtpPcSession TMy;
        
		RtpParams m_prof;		
		boost::scoped_ptr<iLogW::LogSession>  m_log;
        iLogW::LogRecordTag m_infoTag;

        RtpCoreSession m_coreSession;        
        IRtpPcToUser& m_user;     
        bool m_dstAddrIsSet;
        IRtpInfra& m_infra;

        //������� ��� �������� TxData ����� ���������
        struct TxDataParam;

        //������� ��� �������� TxEvent ����� ���������
        struct TxEventParam;

        //������� ��� �������� RxData ����� ���������
        struct RxDataParam;

        //������� ��� �������� RxEvent ����� ���������
        struct RxEventParam;

    // ����������� ��������� User-->Rtp
    private:

        void OnTxData(TxDataParam& param);

        void OnTxEvent(TxEventParam& param);          
        

    //IRtpCoreToUser m_coreSession-->this. �������� �� �����
    private:

        void RxData(BidirBuffer* pBuff, RtpHeaderForUser header);

        void RxEvent(RtpEvent ev, dword timestamp);

        //void RxNewPayload(RtpPayload payload);

        void RtpErrorInd(RtpError err);    

        void NewSsrcRegistred(dword newSsrc);

    // ����������� ��������� Rtp --> User
    private:

        void OnRxData(RxDataParam& param);

        void OnRxEvent(RxEventParam& param);        

        void OnRtpErrorInd(RtpError err);

        void OnNewSsrcRegistred(dword newSsrc);
       

	// User-->RtpPC �������� �� �����, ����������� ��������
	public:

		void TxData(bool marker, dword timestamp, QByteArray data);

		void TxEvent(dword timestamp, RtpEvent ev);

		void setPayload(RtpPayload payloadType);

    public:        

        RtpPcSession(iCore::MsgThread& thread, 
			iLogW::ILogSessionCreator &logCreator,
            const RtpParams& prof, 
            IRtpInfra& infra, 
            IRtpPcToUser& user, 
            const HostInf &dstAddr = HostInf());        
        ~RtpPcSession();
        void setDestAddr(const HostInf &dstAddr);

        RtpStatistic getAllTimeStats() const
        {
            RtpStatistic stat;
            m_coreSession.getAllTimeStats(stat);
            return stat;
        }

        RtpStatistic getLastSsrcStats()  const
        {
            RtpStatistic stat;
            m_coreSession.getLastSsrcStats(stat);
            return stat;
        }

        const HostInf& getLocalRtpAddr() const
        {
            return m_coreSession.getLocalRtpAddr();
        }

        const HostInf& getLocalRtcpAddr() const
        {
            return m_coreSession.getLocalRtcpAddr();
        }
        
    };
} // namespace iRtp

#endif
