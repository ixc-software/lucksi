#include "stdafx.h"
#include "iCore/MsgTimer.h"
#include "SafeBiProtoExt/NObjSbpSettings.h"
#include "TdmMng/TdmException.h"
#include "SafeBiProto/SbpTcpTransportFactory.h"
#include "SafeBiProtoExt/SbpConnection.h" 
#include "NObjWatchdogTest.h"

namespace 
{
	const int CWaitConnection = 60000;
	const int CWaitResponse = 10000;

	Utils::HostInf StringToHostInf(const QString &str)
	{
		return Utils::HostInf::FromString(str.toStdString(), 
			Utils::HostInf::HostAndPort);	
	}

};

namespace iCmpExt
{
	class IOperation : public Utils::IBasicInterface
	{
	public:
		virtual void Activated() = 0;
		virtual void Deactivated(const std::string &) = 0;
		virtual void Process() = 0;
		virtual void MsgReceived(boost::shared_ptr<SBProto::SbpRecvPack> data) = 0;
	};

	// класс выполняет подключение к плате и позволяет отправлять/принимать сообщения
	class NObjWatchdogTest::Connection : boost::noncopyable,
		public iCore::MsgObject,
		public SBProtoExt::ISbpConnectionEvents
	{
		typedef NObjWatchdogTest::Connection T;
	public:
		Connection(NObjWatchdogTest &owner,
			IOperation &user,
			const Utils::HostInf &host)  :
			iCore::MsgObject(owner.getMsgThread()),
			m_user(user),
			m_timer(this, &T::OnProtoProcess)
		{
			SBProtoExt::SbpConnProfile sbpProfile;
			sbpProfile.m_logCreator  = &owner.Log();
			sbpProfile.m_sbpSettings = owner.m_sbpSettings->Settings();
			sbpProfile.m_user = this;
			sbpProfile.SetTransport(SBProto::SbpTcpTransportFactory::CreateTransport(getMsgThread(), host));
			m_sbpConnection.reset(
				new SBProtoExt::SbpConnection(getMsgThread(), sbpProfile, owner.m_sbpSettings->LogSettings()));
			m_sbpConnection->ActivateConnection();
			m_controlTimer.Set(CWaitConnection);
			m_timer.Start(500, true);  // owner.m_sbpSettings->Settings().getTimeoutCheckIntervalMs()
		}

		SBProto::ISafeBiProtoForSendPack &Proto()
		{
			return m_sbpConnection->Proto();
		}

        void Activate()
        {
            m_sbpConnection->ActivateConnection();
        }

        void Deactivate()
        {
            m_sbpConnection->DeactivateConnection();
        }

	// SBProtoExt::ISbpConnectionEvents
	private:
		void CommandReceived(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			boost::shared_ptr<SBProto::SbpRecvPack> data)
		{
			OnMsgReceived(src, data);
		}
		void ResponseReceived(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			boost::shared_ptr<SBProto::SbpRecvPack> data)
		{
			OnMsgReceived(src, data);
		}
		void InfoReceived(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			boost::shared_ptr<SBProto::SbpRecvPack> data)
		{
			OnMsgReceived(src, data);
		}
		void ProtocolError(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			boost::shared_ptr<SBProto::SbpError> err)
		{
			ESS_ASSERT(src.IsEqualIntf(m_sbpConnection.get()));
			
			m_user.Deactivated(err->ToString());
		}
		void ConnectionActivated(Utils::SafeRef<SBProtoExt::ISbpConnection> src)
		{
			ESS_ASSERT(src.IsEqualIntf(m_sbpConnection.get()));			
			m_controlTimer.Stop();
			m_user.Activated();
		}
		void ConnectionDeactivated(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			const std::string &err)
		{
			ESS_ASSERT(src.IsEqualIntf(m_sbpConnection.get()));

			m_user.Deactivated(err);
		}
	private:
		void OnProtoProcess(iCore::MsgTimer *pT)
		{
			if (m_controlTimer.TimeOut()) 
            {
                m_user.Deactivated("Connection timeout");
            }				
			else
			{
				m_sbpConnection->Process();
				m_user.Process();
			}
		}
		void OnMsgReceived(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			boost::shared_ptr<SBProto::SbpRecvPack> data)
		{
			ESS_ASSERT(src.IsEqualIntf(m_sbpConnection.get()));

			m_user.MsgReceived(data);
		}
	private:
		IOperation &m_user;
		iCore::MsgTimer m_timer;
		Utils::TimerTicks m_controlTimer;
		boost::shared_ptr<SBProtoExt::ISbpConnection> m_sbpConnection;
	};

	// -------------------------------------------------------------------------------
	
	class NObjWatchdogTest::GlobalReq : boost::noncopyable,
		public IOperation
	{
	public:
		GlobalReq(NObjWatchdogTest &owner, const Utils::HostInf &host, const iCmp::BfGlobalSetup &msg) : 
			m_owner(owner),
			m_connection(m_owner, *this, host)
		{
			m_msg = msg;
		}
	// IOperation
	private:		
		void Activated()
		{
			iCmp::PcCmdGlobalSetup::Send(m_connection.Proto(), m_msg);	
			m_controlTimer.Set(CWaitResponse);
		}
		void Deactivated(const std::string &desc)
		{
			m_owner.CompleteOperation(desc);
		}
		void Process()
		{
			if(m_controlTimer.TimeOut()) m_owner.CompleteOperation("Wait response timeout.");
		}
		void MsgReceived(boost::shared_ptr<SBProto::SbpRecvPack> data)
		{
			int err = 0;
			std::string desc;
			if(!iCmp::BfResponse::ProcessPacketStatic(*data, err, desc)) return;
			
			if(err == 0)
				m_owner.CompleteOperation("Ok.");
			else
				m_owner.CompleteOperation(iCmp::BfResponse::ResponseToString(err, desc));
		}
	private:
		NObjWatchdogTest &m_owner;
		Utils::TimerTicks m_controlTimer;
		iCmp::BfGlobalSetup m_msg;
		NObjWatchdogTest::Connection m_connection;
	};

	// -------------------------------------------------------------------------------

	class NObjWatchdogTest::StateInfoReq : boost::noncopyable,
		public IOperation
	{
	public:
		StateInfoReq(NObjWatchdogTest &owner, const Utils::HostInf &host) : 
			m_owner(owner),
			m_connection(m_owner, *this, host)
		{}
	// IOperation
	private:		
		void Activated()
		{
			iCmp::PcCmdStateInfo::Send(m_connection.Proto());	
			m_controlTimer.Set(CWaitResponse);
		}
		void Deactivated(const std::string &desc)
		{
			m_owner.CompleteOperation(desc);
		}
		void Process()
		{
			if(m_controlTimer.TimeOut()) m_owner.CompleteOperation("Wait response timeout.");
		}
		void MsgReceived(boost::shared_ptr<SBProto::SbpRecvPack> data)
		{
			iCmp::BfStateInfo info;
			if (iCmp::BfRespStateInfo::ProcessPacketStatic(*data, info)) 	
			{
				m_owner.CompleteOperation(info.Stats.ToString("\n"));
				return;
			}
			int err = 0;
			std::string desc;
			if(!iCmp::BfResponse::ProcessPacketStatic(*data, err, desc)) return;
			
			m_owner.CompleteOperation(iCmp::BfResponse::ResponseToString(err, desc));
		}
	private:
		NObjWatchdogTest &m_owner;
		Utils::TimerTicks m_controlTimer;
		NObjWatchdogTest::Connection m_connection;
	};

    // -------------------------------------------------------------------------------

    // bad IOperation design, too many copy-paste
    class NObjWatchdogTest::WatchdogLoop : boost::noncopyable,
        public IOperation
    {
        enum State
        {
            StWaitActivation,
            StWaitDeactivation,
            StActive,
        };

        int m_times;
        int m_execTime;
        State m_state;
        NObjWatchdogTest &m_owner;
        NObjWatchdogTest::Connection m_connection;

        void FixState(State state)
        {
            m_state = state;
            m_execTime = Platform::GetSystemTickCount();
        }

        void Log(const std::string &msg)
        {
            // std::cout << msg << std::endl;
        }

    // IOperation
    private:	

        void Activated()
        {
            Log("Activated");

            iCmp::BfGlobalSetup msg;
            msg.WatchdogTimeoutMs = 1;

            iCmp::PcCmdGlobalSetup::Send(m_connection.Proto(), msg);	

            FixState(StActive);
        }

        void Deactivated(const std::string &desc)
        {
            Log("Deactivated");

            --m_times;

            if (m_times <= 0)
            {
                m_owner.CompleteOperation("Done!");
                return;
            }

            FixState(StWaitActivation);
            
            m_connection.Activate();
        }

        void Process()
        {
            int time = Platform::GetSystemTickCount() - m_execTime;

            if (m_state == StActive)
            {
                if (time > 1000) 
                {
                    Log("Force deactivation");
                    m_connection.Deactivate();
                    FixState(StWaitDeactivation);
                    Deactivated("");  // manual call
                }
            }
            else if (m_state == StWaitActivation)
            {
                if (time > 30 * 1000)
                {
                    m_owner.CompleteOperation("Wait activation timeout, remains " + Utils::IntToString(m_times));     
                }
            }
        }

        void MsgReceived(boost::shared_ptr<SBProto::SbpRecvPack> data)
        {
            // nothing
        }

    public:

        WatchdogLoop(NObjWatchdogTest &owner, const Utils::HostInf &host, int times) : 
          m_times(times), m_owner(owner), m_connection(m_owner, *this, host)
        {
            ESS_ASSERT(m_times > 0);

            FixState(StWaitActivation);
        }

    };


} // namespace iCmpExt

// -------------------------------------------------------------------------------

namespace iCmpExt
{
    using namespace SBProtoExt;

	NObjWatchdogTest::NObjWatchdogTest(Domain::IDomain *pDomain, const Domain::ObjectName &name) : 
		Domain::NamedObject(pDomain, name),
		m_sbpSettings(new SBProtoExt::NObjSbpSettings(this))
	{
	}

	// -------------------------------------------------------------------------------

	NObjWatchdogTest::~NObjWatchdogTest()
	{
	}

	// -------------------------------------------------------------------------------

	void NObjWatchdogTest::SmallWatchdogTest(DRI::IAsyncCmd *pAsyncCmd, const QString &addr)
	{
		iCmp::BfGlobalSetup val;
		val.WatchdogTimeoutMs = 1;
		StartGlobalOperation(pAsyncCmd, StringToHostInf(addr), val);
	}

	// -------------------------------------------------------------------------------

	void NObjWatchdogTest::HugeWatchdogTest(DRI::IAsyncCmd *pAsyncCmd, const QString &addr)
	{
		iCmp::BfGlobalSetup val;
		val.WatchdogTimeoutMs = 1000 * 1000;
		StartGlobalOperation(pAsyncCmd, StringToHostInf(addr), val);
	}

	// -------------------------------------------------------------------------------
	
	void NObjWatchdogTest::LoopForeverTest(DRI::IAsyncCmd *pAsyncCmd, const QString &addr)
	{
		iCmp::BfGlobalSetup val;
		val.WatchdogTimeoutMs = 5 * 1000;
		val.DoLoopForever = true;
		StartGlobalOperation(pAsyncCmd, StringToHostInf(addr), val);
	}
	
	// -------------------------------------------------------------------------------	

	void NObjWatchdogTest::AssertTest(DRI::IAsyncCmd *pAsyncCmd, const QString &addr)
	{
		iCmp::BfGlobalSetup val;
		val.DoAssert = true;
		val.ResetOnFatalError = true;
		StartGlobalOperation(pAsyncCmd, StringToHostInf(addr), val);
	}

	// -------------------------------------------------------------------------------

	void NObjWatchdogTest::OutOfMemoryTest( DRI::IAsyncCmd *pAsyncCmd, const QString &addr )
	{
		iCmp::BfGlobalSetup val;

		val.ResetOnFatalError = true;
		val.DoOutOfMemory = true;

		StartGlobalOperation(pAsyncCmd, StringToHostInf(addr), val);
	}

    // -------------------------------------------------------------------------------

    void NObjWatchdogTest::EchoHaltTest( DRI::IAsyncCmd *pAsyncCmd, const QString &addr )
    {
        iCmp::BfGlobalSetup val;

        val.DoEchoHalt = true;

        StartGlobalOperation(pAsyncCmd, StringToHostInf(addr), val);
    }

	// -------------------------------------------------------------------------------

	void NObjWatchdogTest::StateInfo(DRI::IAsyncCmd *pAsyncCmd, const QString &addr)
	{
        Utils::HostInf hi = StringToHostInf(addr);  // try convert before async begin

		AsyncBegin(pAsyncCmd);		
		
		m_operation.reset(new StateInfoReq(*this, hi));
	}

    // -------------------------------------------------------------------------------

    void NObjWatchdogTest::RunWatchdogLoop( DRI::IAsyncCmd *pAsyncCmd, const QString &addr, int times )
    {
        if (times <= 0) ThrowRuntimeException("Bad 'times'!");

        Utils::HostInf hi = StringToHostInf(addr);  // try convert before async begin

        AsyncBegin(pAsyncCmd);		

        m_operation.reset( new WatchdogLoop(*this, hi, times) );
    }

	// -------------------------------------------------------------------------------

	void NObjWatchdogTest::OnDeleteConnection(boost::shared_ptr<Utils::IBasicInterface>)
	{
	}

	// -------------------------------------------------------------------------------

	void NObjWatchdogTest::StartGlobalOperation(DRI::IAsyncCmd *pAsyncCmd, const Utils::HostInf &addr, 
		const iCmp::BfGlobalSetup &msg)
	{
		AsyncBegin(pAsyncCmd);
		m_operation.reset(new GlobalReq(*this, addr, msg));
	}

	// -------------------------------------------------------------------------------

	void NObjWatchdogTest::CompleteOperation(const std::string &desc)
	{
		PutMsg(this, &T::OnDeleteConnection, m_operation);		
		m_operation.reset();
		AsyncOutput(desc.c_str());
		AsyncComplete(true);
	}

}

