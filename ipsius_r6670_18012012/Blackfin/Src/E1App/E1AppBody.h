#ifndef _E1_APP_BODY_H_
#define _E1_APP_BODY_H_

#include "Platform/Platform.h"
#include "Utils/BidirBuffer.h"
#include "Utils/SafeRef.h"
#include "iLog/LogWrapper.h"
#include "iCmp/ChMngProto.h"
#include "TdmMng/TdmManager.h"
#include "Ds2155/boardds2155.h"
#include "ChipSecure/ChipVerify.h"
#include "DrvAoz/AozShared.h"
#include "psbCofidec/IPsbDebug.h"

#include "IEchoClient.h"

namespace DrvAoz
{
    class AozBoard;
}

namespace SBProto	
{	
	class SbpRecvPack;	
	class SafeBiProto;
}

namespace SBProtoExt
{	
	class ISbpConnection;
}

namespace PsbCofidec
{
    class PsbCofidecDrv;
}

namespace E1App
{
    using boost::scoped_ptr;

	class E1AppConfig;
    class EchoSfxClient;

    struct E1BodyParams
    {
        iCore::MsgThread &Thread;
        E1App::E1AppConfig AppConfig;
        Utils::SafeRef<iLogW::ILogSessionCreator> LogCreator;
        IEchoClient &Echo;
        Utils::SafeRef<SBProtoExt::ISbpConnection> Connection;

        E1BodyParams(iCore::MsgThread &thread,
            E1App::E1AppConfig appConfig,
            Utils::SafeRef<iLogW::ILogSessionCreator> logCreator,
            IEchoClient &echo,
            Utils::SafeRef<SBProtoExt::ISbpConnection> connection) :
            Thread(thread),
            AppConfig(appConfig),
            LogCreator(logCreator),
            Echo(echo),
            Connection(connection)
        {
        }

    };

    // abstract class; TdmMng::TdmManager owner, partial IBfSideRecv impl
	class E1AppBody : boost::noncopyable,
        // public iCore::MsgObject,   // Bf crush 
		public iCmp::IBfSideRecv,
        public TdmMng::IIEventsQueueItemDispatch
	{
	public:

        virtual ~E1AppBody();

        void SendInitResp();
		void Process();
		void LogState() const;
		void ProcessCmd(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			SBProto::SbpRecvPack &data);  // can throw TdmException;

		bool IsActiveConnection(Utils::SafeRef<SBProtoExt::ISbpConnection> connection) const
        {
            return (connection == m_connection);
        }

        std::string ActiveConnectionInfo() const
        {
            return m_connection->TransportInfo();
        }

        int ActiveChannelsCount() const
        {
            return m_pStream->ActiveChannelsCount();
        }

        bool DeviceState()
        {
            return GetDeviceState();
        }

        std::string ProtectionInfo() const
        {
            std::ostringstream oss;

            if (m_secureChip != 0)
            {
                oss << m_secureChip->StatInfo() << " bs " << m_blockSend;
            }
            else
            {
                oss << "Disabled";
            }

            return oss.str();
        }

        static void InitAozSync()
        {
            if ( DevIpTdm::AozInit::InitDone() ) return;

            if (!DevIpTdm::AozInit::InitSync()) 
            {
                ESS_THROW_T(TdmMng::TdmException, TdmMng::terAozExtraInitFail);
            }
        }

    protected:

        E1AppBody(E1BodyParams &params,
            const iCmp::BfInitDataBase &data,
            bool enableT1Mode);

        void HeapLog(const std::string &desc, 
            const std::string &header = "Heap/log", bool addHeapInfo = true) const;

        void SendRespOK();

        SBProto::ISafeBiProtoForSendPack& Proto()
        {
            return m_connection->Proto();
        }

        TdmMng::TdmStream& FindStream(const string &devName);

        TdmMng::TdmHAL& HAL()
        {
            ESS_ASSERT(m_hal != 0);
            return *m_hal;
        }

        void SendBfTdmEvent(const TdmMng::TdmAsyncEvent &e);

        void StartProtection();

        void ThrowFromInit(const iCmp::BfInitDataBase &data);

    // TdmMng::IIEventsQueueItemDispatch impl
    private:

        void OnAozLineEvent(const std::string &devName, int chNum, TdmMng::AozLineEvent e);

	// iCmp::IBfSideRecv impl
	private:	
		void OnPcCmdStartGen(const string &devName, int chNum, bool toLine,
			const iDSP::Gen2xProfile &data);
		void OnPcCmdStopGen(const string &devName, int chNum, bool toLine);

        void OnPcCmdChEchoSuppressMode(const string &devName, int chNum, int taps, bool useInternal);
        void OnPcCmdChCaptureMode(const string &devName, int chNum, bool enabled);

        // void OnPcCmdLogSetup(bool logEnable, const string &udpHostPort);

		void OnPcCmdGetRtpPort(const string &devName, int chNum);
		void OnPcCmdGetTdmInfo(const string &devName);
		void OnPcCmdGetChannelInfo(const string &devName, int chNum, bool lastCall);

		void OnPcCmdStartRecv(const string &devName, int chNum);
		void OnPcCmdStopRecv(const string &devName, int chNum);

		void OnPcCmdStartSend(const string &devName, int chNum, 
			const std::string &ip, int port);
		void OnPcCmdStopSend(const string &devName, int chNum, 
			const std::string &ip, int port);

        void OnPcCmdSetFreqRecvMode(const string &devName, int chMask, bool useInternal,
            const string &recvName, iCmp::FreqRecvMode mode, const string &params);

        // conf related 
        void OnPcCmdConfCreate(int confHandle, bool autoMode, int blockSize);
        void OnPcCmdConfDelete(int confHandle);
        void OnPcCmdConfMode(int confHandle, const string &mode);
        void OnPcCmdConfCreateTdmPoint(int pointHandle, const string &devName, int chNum);
        void OnPcCmdConfCreateRtpPoint(int pointHandle, const string &codecName, const string &codecParams);
        void OnPcCmdConfCreateGenPoint(int pointHandle, const iDSP::Gen2xProfile &profile);
        void OnPcCmdConfDeletePoint(int pointHandle);
        void OnPcCmdConfRtpPointSend(int pointHandle, const string &ip, int port);
        void OnPcCmdConfAddPointToConf(int pointHandle, int confHandle, bool send, bool recv);
        void OnPcCmdConfRemovePointFromConf(int pointHandle);
        void OnPcCmdConfGetState(int confHandle);

        // unsupported, E1 only
        void OnPcCmdGetLiuInfo(const string &devName) { ThrowUnsupportedCommand(); }
        void OnPcHdlcSend(/* const string &devName, */ std::vector<Platform::byte> &pack)
        {
            ThrowUnsupportedCommand();
        }

        // unsupported, AOZ only
        void OnPcCmdAbSetCofidecSetup(const string &devName, int chMask, const iCmp::AbCofidecParams &params)
        {
            ThrowUnsupportedCommand();
        }

        void OnPcCmdAbSetLineParams(const string &devName, int chMask, const iCmp::AbLineParams &params)
        {
            ThrowUnsupportedCommand();
        }

        void OnPcCmdAbCommand(const string &devName, int chMask, const string &cmdName, const string &cmdParams)
        {
            ThrowUnsupportedCommand();
        }

        // unsupported, Pult only
        void OnPcCmdPultCofidec(const string &devName, const iPult::CofidecState &state)
        {
            ThrowUnsupportedCommand();
        }


	private:

		TdmMng::ITdmChannel &FindChannel(const string &devName, int chNum);

        // void AozEventsHook(const std::string &devName, const DrvAoz::AozEvent &e);
        void StopGenToLine(const std::string &devName, int chNum);
        void ThrowUnsupportedCommand();

        virtual void DoProcess() 
        {
            // nothing, for override
        }

        virtual void PrintLogState() const
        {
            // nothing, for override
        }

        virtual bool GetDeviceState() = 0;

	private:
		iCmp::BfRecv m_cmpRecv;
		scoped_ptr<iLogW::LogSession> m_logSession;
		iLogW::LogRecordTag m_tagInfo;

		Utils::SafeRef<SBProtoExt::ISbpConnection> m_connection;

        scoped_ptr<TdmMng::TdmHAL> m_hal;
		scoped_ptr<TdmMng::TdmManager>  m_tdm;     // TDM/RTP
		TdmMng::TdmStream*      m_pStream;

        IEchoClient &m_echo;

        // secure
        scoped_ptr<ChipSecure::ChipVerify> m_secureChip;
        bool m_blockSend;
        int m_startSendCounter;
	};

    // -------------------------------------------------

    // Ds2155::BoardDs2155 + Ds2155::Hdlc owner, E1 specific stuff
    class E1AppBodyE1 : public E1AppBody
    {
        scoped_ptr<Ds2155::BoardDs2155> m_board;   // LIU/HDLC
        scoped_ptr<Ds2155::Hdlc>        m_hdlc;
        bool                            m_e1Activated;

        void DoProcess();               // override
        void PrintLogState() const;     // override
        bool GetDeviceState();          // override

        Utils::BidirBuffer *PeekHdlcPack();

    // iCmp::IBfSideRecv partial impl
    private:	

        void OnPcHdlcSend(/* const string &devName, */ std::vector<Platform::byte> &pack);
        void OnPcCmdGetLiuInfo(const string &devName);

    public:

        E1AppBodyE1(E1BodyParams &params,
            const iCmp::BfInitDataE1 &data);

    };

    // -------------------------------------------------

    // list of DrvAoz::AozBoard
    class E1AppBodyAoz : 
        public E1AppBody, 
        public TdmMng::ITdmHalEvents,
        public virtual Utils::SafeRefServer  // is it OK on blackfin - ?
    {
		boost::scoped_ptr<iLogW::LogSession> m_log;
        Utils::ManagedList<DrvAoz::AozBoard> m_boards;

        DrvAoz::AozBoard& Board(const string &name);

        bool GetDeviceState();          // override

    // TdmMng::ITdmHalEvents impl
    private:

        void OnTdmHalIrq(int sportNum, TdmMng::ITdmBlock &block);
        void OnTdmHalBlockCollision();

    // iCmp::IBfSideRecv partial impl
    private:	

        void OnPcCmdAbSetCofidecSetup(const string &devName, int chMask, const iCmp::AbCofidecParams &params);
        void OnPcCmdAbSetLineParams(const string &devName, int chMask, const iCmp::AbLineParams &params);
        void OnPcCmdAbCommand(const string &devName, int chMask, const string &cmdName, const string &cmdParams);

    public:

        E1AppBodyAoz(E1BodyParams &params,
            const iCmp::BfInitDataAoz &data);

        ~E1AppBodyAoz();

    };

    // -------------------------------------------------

    class E1AppBodyPult : 
        public E1AppBody,
        public PsbCofidec::IPsbDebug
    {

        class Profiler;

        const std::string m_devName;
        boost::scoped_ptr<iLogW::LogSession> m_log;   // Bf: vs exception trouble 
        boost::scoped_ptr<PsbCofidec::PsbCofidecDrv> m_drv;

        bool m_cofidecBugCapture;
        std::string m_lastCofidecError;

        bool GetDeviceState();          // override

        void StartDebugMode();
        void ProcessCofidecError(const std::exception &e);

    // PsbCofidec::IPsbDebug impl
    private:

        void WriteLn(const std::string &line)
        {
            HeapLog(line, "Cofidec", false);
        }

    // iCmp::IBfSideRecv partial impl
    private:	

        void OnPcCmdPultCofidec(const string &devName, const iPult::CofidecState &state);

    public:

        E1AppBodyPult(E1BodyParams &params,
            const iCmp::BfInitDataPult &data);

        ~E1AppBodyPult();

    };

};

#endif

