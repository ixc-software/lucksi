#ifndef __TDMCHANNEL__
#define __TDMCHANNEL__

#include "Utils/BidirBuffer.h"
#include "Utils/IBasicInterface.h"
#include "Utils/HostInf.h"
#include "Utils/IVirtualDestroy.h"
#include "iCmp/ChMngProto.h"
#include "DrvAoz/AozShared.h"

#include "ITdmGenerator.h"
#include "ITdmManager.h"
#include "IFreqRecvOwner.h"
#include "RtpToTdmBuff.h"
#include "ITdmSource.h"
#include "TdmToTdmBuff.h"
#include "TdmChannelSender.h"
#include "TdmChannelReceiver.h"
#include "TdmChannelStatistic.h"
#include "tdmdatacapture.h"


namespace FreqRecv
{
    class FreqRecvPoolLocal;
}

namespace TdmMng
{
    using boost::scoped_ptr;

    // -------------------------------------------------------------

    // user interface for channel
    // any method can throw TdmException
    class ITdmChannel : public Utils::IBasicInterface
    {
    public:

        virtual void StartGen(bool toLine, const iDSP::Gen2xProfile &data) = 0;
        virtual void StopGen(bool toLine) = 0;

        virtual void EchoSuppressMode(int taps) = 0;
        virtual void DataCaptureModeMode(bool enabled) = 0;

        virtual int GetRtpPort() const = 0;

        virtual void StartRecv() = 0;
        virtual void StopRecv() = 0;

        virtual void StartSend(const std::string &ip, int port) = 0;
        virtual void StopSend(const std::string &ip, int port) = 0;

        virtual void SetFreqRecvMode(const std::string &recvName, const std::string &params, iCmp::FreqRecvMode mode) = 0;

        // object info                 
        virtual void GetAllTimeStats(TdmChannelStatistic& statOut) const = 0;
        virtual void GetLastCallStats(TdmChannelStatistic& statOut) const = 0;                

        virtual int  GetChNumber() const = 0;
        virtual void RouteAozEvent(TdmMng::AozLineEvent e) = 0;

        // conference
        virtual void BindToConference(const Utils::SafeRef<ISendSimple> &send, 
            const Utils::SafeRef<IRtpLikeTdmSource> &recv) = 0;
        virtual void UnbindToConference(const Utils::SafeRef<ISendSimple> &send,
            const Utils::SafeRef<IRtpLikeTdmSource> &recv) = 0;
               
    };

    // -------------------------------------------------------------
   

    // single TDM channel
    class TdmChannel : 
        public ITdmChannel, 
        public iRtp::IRtpCoreToUser,
        public IFreqRecvOwner,
        boost::noncopyable
    {        
        // struct LogTags; // doesn't compile in GCC
		ITdmOwner &m_owner;
		ITdmManager &m_mng;
        const std::string m_streamName;
        const int m_number;  // timeslot

		// log fields:
		scoped_ptr<iLogW::LogSession> m_log; 
		iLogW::LogRecordTag m_tagChannelControl;
		iLogW::LogRecordTag m_tagRWData;

        bool m_useAlaw;
        const iDSP::ICodec &m_codec;
        int m_lastPollTime;

        iRtp::RtpCoreSession m_rtp;

        TdmChannelReceiver m_recv;
        TdmChannelSender m_send;

        const std::string m_eventSrc;

        // generators
        scoped_ptr<ITdmGenerator> m_genFromTdm;
        scoped_ptr<ITdmGenerator> m_genToTdm;

        // data capture
        scoped_ptr<TdmDataCapture> m_dataCapture;

        scoped_ptr<FreqRecv::FreqRecvPoolLocal> m_freqRecvs;        

        void FillReadBlock(TdmReadWrapper &block, Utils::BidirBuffer *pBuff); // no throw

    // IFreqRecvOwner impl
    private:

        void PushDialBeginEvent(const std::string &devName, int chNum);
        void PushFreqRecvEvent(const std::string &devName, int chNum,
            const std::string &freqRecvName, const std::string &data);

        const iDSP::ICodec& GetChCodec() const { return m_codec; }

        FreqRecv::ITdmFreqRecvLocal* CreateFreqRecv(const std::string &name, 
            const std::string &params);

        Utils::BidirBuffer* CreateBidirBuffer();

    // iRtp::IRtpCoreToUser impl
    private:

        void RxData(BidirBuffer* pBuff, iRtp::RtpHeaderForUser header);
        void RxEvent(iRtp::RtpEvent ev, dword timestamp);
        void RtpErrorInd(iRtp::RtpError er);
        void NewSsrcRegistred(Platform::dword ssrc);

    // ITdmChannel impl
    private:

        void StartGen(bool toLine, const iDSP::Gen2xProfile &data);
        void StopGen(bool toLine);

        int GetRtpPort() const;

        void StartRecv();
        void StopRecv();

        void StartSend(const std::string &ip, int port);
        void StopSend(const std::string &ip, int port);

        void EchoSuppressMode(int taps);
        void DataCaptureModeMode(bool enabled);

        void SetFreqRecvMode(const std::string &recvName, const std::string &params, iCmp::FreqRecvMode mode);

        void GetAllTimeStats(TdmChannelStatistic& statOut) const
        {
            m_recv.GetAllTimeStat(statOut);
            m_rtp.getAllTimeStats(statOut.RtpStat);
        }

        void GetLastCallStats(TdmChannelStatistic& statOut) const
        {
            m_recv.GetLastCallStat(statOut);
            m_rtp.getLastSsrcStats(statOut.RtpStat);
        }               

        int GetChNumber() const
        {
            return m_number;
        }

        void RouteAozEvent(TdmMng::AozLineEvent e);

        void BindToConference(const Utils::SafeRef<ISendSimple> &send, 
                              const Utils::SafeRef<IRtpLikeTdmSource> &recv)
        {
            // bad invariant on exception - ?
            // ... 

            m_recv.Start(recv);  
            m_send.StartSend(send);
        }

        void UnbindToConference(const Utils::SafeRef<ISendSimple> &send,
                                const Utils::SafeRef<IRtpLikeTdmSource> &recv)
        {
            m_recv.Stop(recv);
            m_send.StopSend(send);
        }
        
    public:
      
        TdmChannel(ITdmOwner &owner, 
			Utils::SafeRef<iLogW::ILogSessionCreator> logCreator,
			ITdmManager &mng, int number, 
            bool useAlaw, const iDSP::ICodec &codec, const std::string &streamName);

	    ~TdmChannel();

        int Number() const { return m_number; }

        void TdmRead(TdmReadWrapper &block);
        void TdmWrite(TdmWriteWrapper &block);

        int RtpPort() const;

        ITdmIn& GetTdmInput();

        void SocketPoll(int currTime, Utils::StatElementForInt &sockStat);

        // recv or send enabled
        bool IsActive();

    };
		
}  // namespace TdmMng

#endif
