#include "stdafx.h"
#include "BootServer.h"
#include "BroadcastAutoSender.h"
#include "E1App/MngLwip.h"
#include "SafeBiProtoExt/SbpConnection.h"
#include "SafeBiProtoExt/SbpUartTransportFactory.h"
#include "BfDev/BoardWathcdog.h"
#include "BfBootCore/BooterVersion.h"
#include "BoardSetup.h"
#include "SafeBiProtoExt/SbpUartTransportFactory.h"
#include "BfBootCore/GeneralBooterConst.h"
#include "DevIpTdm/BfLed.h"

namespace 
{  
    const Platform::dword CProcessMsec = 100; // --- < watchdogPeriod ?
    const Platform::dword CTryStartNetMsec = 400; // --- < watchdogPeriod ?
    //const Platform::dword CNextCmdMsec = 4 * 1000;
    const bool CDebugTraceCmd = false;           
} // namespace 

// ------------------------------------------------------------------------------------

namespace BfBootSrv
{   
    using iLogW::EndRecord;
    using BfBootCore::ClientRespLogin;     
    using BfBootCore::ClientRespCmd;     
    //using BfBootCore::CBooterVersion;         

    // выполняет подмешивание информации об источнике при обработке команд ILogin
    class BootServer::LoginAdapter : public BfBootCore::ILogin 
    {
        BfBootCore::LoginConv m_loginConverter;
        BootServer& m_callback;    
        Utils::SafeRef<SBProtoExt::ISbpConnection>& m_src;

    // ILogin
    private:                
        void DoLogin(const std::string& pwd, dword protoVersion)
        {            
            m_callback.Login(m_src, pwd, protoVersion);
        }

        void DoLogout(bool withSave)
        {
            m_callback.Logout(m_src, withSave);
        }

        void SetTrace(bool on)
        {
            m_callback.SetTrace(m_src, on);
        }

        void RunAditionalScript(const std::string& scriptName, bool withSaveChanges)
        {
            m_callback.RunAditionalScript(m_src, scriptName, withSaveChanges);
        }

    public:

        LoginAdapter(BootServer& callback, Utils::SafeRef<ISbpConnection>& src) 
            : m_loginConverter(*this), 
            m_callback(callback),
            m_src(src)
        {}


        //return true only if ILogin protocol command
        bool ProcessCmd(SbpRecvPack &pack)
        {
            return m_loginConverter.Process(pack);
        }
    };

    // ------------------------------------------------------------------------------------

    BootServer::CheckParams::CheckParams( const BootServerParams& params )
    {
        ESS_ASSERT(params.IsValid());
    }

    // ------------------------------------------------------------------------------------

    void BootServer::InitUart()      
    {             
        if (m_params.COM == BootServerParams::Unused) return;
        
        if(m_log->LogActive()) *m_log << "CreateUart..." << EndRecord;

        SBProtoExt::SbpUartProfile prof;
        prof.BoudRate = BfBootCore::CComRate;
        std::string err;
        boost::shared_ptr<SBProto::ISbpTransport> transport =
            SBProtoExt::SbpUartTransportFactory::CreateTransport(m_params.COM, prof, err, true);            

        if (transport)
        {
            SBProtoExt::SbpConnProfile prof;
            prof.m_logCreator = m_log->LogCreator();   
            prof.SetTransport( transport );
            prof.m_user = this;
            prof.m_sbpSettings = m_params.SbpProf;
            m_uartConnection.reset(new SBProtoExt::SbpConnection(m_runner.getThread(), prof, m_sbpLogParam));
            m_uartConnection->ActivateConnection();
        }
        else
        {
            if(m_log->LogActive()) *m_log << "Can`t create UartTransport. Reason: " << err << EndRecord; 
        }                      
        
        if(m_log->LogActive()) *m_log << "UartCreated." << EndRecord;  

    } 

    // ------------------------------------------------------------------------------------

    void BootServer::InitNetworkTools()
    {
        ESS_ASSERT(E1App::Stack::IsInited()
            && E1App::Stack::Instance().IsEstablished()
            && !E1App::Stack::Instance().IpIsEmpty()
            );        

        SbpTcpServer::Profile prof;            
        prof.m_connectionProfile.m_logCreator = m_log->LogCreator();            
        prof.m_connectionProfile.m_sbpSettings = m_params.SbpProf;		
        prof.m_tcpPort = m_params.CbpPort;                            
        prof.m_logSettings = m_sbpLogParam;
        prof.m_user = this;            

        m_tcpSbp.reset(new SbpTcpServer(getMsgThread(), prof) );                                                        

        if (m_params.Cfg.CanCreateBroadcast())
        {            
            BfBootCore::BroadcastMsg msg = m_params.Cfg.CreateBroadcastForBooter(m_tcpSbp->LocalHostInf().Port());
            m_broadcast.reset(new BfBootSrv::BroadcastAutoSender(getMsgThread(), msg, m_params.BroadcastSrcPort));
            m_broadcast->Start(0);
        }        

        m_bootControl.NetworkActivated();
    }

    // ------------------------------------------------------------------------------------

    BootServer::BootServer( iCore::IThreadRunner& runner, BootServerParams& params ) 
        : MsgObject(runner.getThread()),
        m_paramsOk(params), 
        m_params(params),
        m_runner(runner),
        m_logMng(params.LogSettings),        
        m_log(m_logMng.CreateSession("BootServer", params.CreteTraceActive) ),
        m_tagObjInfo(m_log->RegisterRecordKindStr("ServerInfo")),        
        m_bootControl(runner.getThread(), *this, params.Action, params.Cfg, m_log->LogCreator()),
        m_prcTimer(this, &BootServer::OnProcess),
        m_connectionList(runner.getThread(), this),
        m_startNetTimer(this, &BootServer::OnStartNetwork)        
    {   
        // todo Разделить конструктор на InitXXX()...                                 

        m_sbpLogParam.reset( new SBProtoExt::SbpLogSettings(0, 5) );                   

        // проверять params.Cfg.UserPwd.IsInited() при инициализации стека а здесь ассерт?
        if (E1App::Stack::IsInited() /*&& params.Cfg.IsExistPwd() - ?*/)            
        {        
            if (E1App::Stack::Instance().Started()) InitNetworkTools();
            else 
                if (!TryStartNetwork()) m_startNetTimer.Start(CTryStartNetMsec);
        }        

        InitUart();
        
		
        m_serverInfo = (params.Cfg.IsServiceMode()) ? 
			"Mode: service." :	"Mode: user.";
		
		if(m_log->LogActive())
		{
			*m_log << "BootServer created" << EndRecord;
			*m_log << m_tagObjInfo << m_serverInfo << EndRecord;
		}
        SetTrace(Utils::SafeRef<ISbpConnection>(), params.CreteTraceActive);                

        m_prcTimer.Start(CProcessMsec, true);
    }

	// ------------------------------------------------------------------------------------

	BootServer::~BootServer()
	{                      
		if(m_log->LogActive()) *m_log << "BootServer delete." << EndRecord;
	}

    // ------------------------------------------------------------------------------------

    bool BootServer::TryStartNetwork()
    {    	
        E1App::Stack& stack = E1App::Stack::Instance();
    	
        if (!stack.IsEstablished()) return false;
        *m_log << "Stack physic layer is established." << EndRecord;
                
        if (!stack.Started()) 
        {
            stack.Start();
            *m_log << "TryStartNetwork..." << EndRecord;
        }

        if (stack.IpIsEmpty()) return false;

        InitNetworkTools();
        *m_log << "Network Started." << EndRecord;
        return true;
    }

    // ------------------------------------------------------------------------------------

    void BootServer::NewConnection( const SBProtoExt::SbpTcpServer* pSrv, boost::shared_ptr<SBProtoExt::ISbpConnection> link )
    {
        ESS_ASSERT(pSrv == m_tcpSbp.get());
        // set callback
        //link->BindUser(this);
        m_connectionList.Push(link);

        *m_log << "New tcp-connection has received. Connected from " << link->Name() << EndRecord;
    }

    // ------------------------------------------------------------------------------------         

    void BootServer::ProcessProtocolError( Utils::SafeRef<ISbpConnection> src, const std::string &errTxt )
    {
        *m_log << "Protocol error: " << errTxt << " Source info: " << src->Name() << EndRecord;
        if (!m_active.IsEmpty() && src == m_active) Disconnect();

        // if uart err
        if ( src.IsEqualIntf(m_uartConnection.get()) )
        {        
            *m_log << "Uart transport was recreated..." << EndRecord; // -- ????
            PutMsg(this, &BootServer::OnUartRestart);            
        }
    }

    // ------------------------------------------------------------------------------------

    void BootServer::ProtocolError( Utils::SafeRef<ISbpConnection> src, boost::shared_ptr<SBProto::SbpError> err )
    {        
        ProcessProtocolError(src, err->ToString());                     
    }

    // ------------------------------------------------------------------------------------

    void BootServer::OnUartRestart()
    {        
        m_uartConnection.reset();
        InitUart();
    }

    // ------------------------------------------------------------------------------------

    void BootServer::CommandReceived( Utils::SafeRef<ISbpConnection> src, boost::shared_ptr<SBProto::SbpRecvPack> data )
    {   
        /*блокировать UART-лог сразу и снимать блокировку если при выходе он не активный?
         Сейчас блокируется только если это команда логин. Управление бблокировкой через raii?*/

        if (CDebugTraceCmd) std::cout << "CmdReceived: " << (*data)[0].AsString() << std::endl;                
        
        LoginAdapter login(*this, src);

        
        if (login.ProcessCmd(*data)) return;

		// Not cmd ILogin

        *m_log << "Server control command received from " << src->Name() << EndRecord;            

        if (m_active.IsEmpty()) return ProcessProtocolError(src, "Expect login command first from new connection.");      
		
        try
        {                   
            if (!m_bootControl.ProcessData(*data)) ProcessProtocolError(src, "Invalid command");
        }
        catch(BfBootCore::StringToTypeConverter::CantConvert&)
        {
            ProcessProtocolError(src, "Invalid command");
        }                
       
    }

    // ------------------------------------------------------------------------------------

    void BootServer::SetTrace(Utils::SafeRef<ISbpConnection> src, bool on )
    {
        if (!m_active.IsEmpty() && src != m_active)
        {
            // ClientRespCmd::Send(src->Proto(), BfBootCore::errServerBusy);
            return;
        }

        if ( E1App::Stack::IsInited() && E1App::Stack::Instance().Started() )
        {
			m_log->LogActive(on);

            if(m_log->LogActive()) 
            {
                *m_log << "Trace turn on." << EndRecord;
                *m_log << m_tagObjInfo << m_serverInfo << EndRecord;
            }
        }
        

        if (!src.IsEmpty()) ClientRespCmd::Send(src->Proto(), BfBootCore::Ok);
    }
    
    // ------------------------------------------------------------------------------------

    void BootServer::Login( Utils::SafeRef<ISbpConnection> src, const std::string& pwd, dword protoVersion )
    {        
        bool isSrcUart = src.IsEqualIntf(m_uartConnection.get());
        //if ( isSrcUart ) LockUartLog();        
        
        *m_log << "Received CmdLogin from " << src->Name();     

        // Wrong protocol version.
        if (protoVersion != BfBootCore::CProtoVersion) 
        {
            std::ostringstream info;
            info << "Booter protoVersion = " << BfBootCore::CProtoVersion;
            ClientRespLogin::Send(
                src->Proto(), BfBootCore::errWrongProtoVersion, info.str(),
                BfBootCore::BooterVersionInfo(), !m_params.Cfg.IsServiceMode()
                );
            //if ( isSrcUart ) UnlockUartLog();
            return;
        }

        // wrong password
        //if (!isSrcUart && m_params.Cfg.IsExistPwd() && m_params.Cfg.getPwd() != pwd)
        if (!isSrcUart && !m_params.Cfg.ValidatePwd(pwd))
        {
            ClientRespLogin::Send(
                src->Proto(), BfBootCore::errWrongPassword, "",
                BfBootCore::BooterVersionInfo(), !m_params.Cfg.IsServiceMode()
                ); // response
            return;
        }

        // pwd ok, do connection.                        
        //bool loginComplete = false;
        BfBootCore::UserResponseCode respCode;
        std::string additionalInfo;
        if (m_active.IsEmpty()) 
        {
            //loginComplete = true;
            respCode = BfBootCore::Ok;
            m_active = src;
            m_bootControl.Connect(src);     
            if (!isSrcUart) m_broadcast->UpdateOwnerInfo( src->TransportInfo() );
            //if (m_activeBroadcast) m_activeBroadcast->Stop();
            *m_log << "Login Complete" << EndRecord;
        }
        else // pwd ok, server busy
        {       
            respCode = BfBootCore::errServerBusy;            
            additionalInfo = m_active->Name();            
            *m_log << "Login Failed. Server already have active connection: " << additionalInfo << EndRecord;
        }

        ClientRespLogin::Send(
            src->Proto(), respCode,
            additionalInfo, BfBootCore::BooterVersionInfo(), 
            !m_params.Cfg.IsServiceMode()
            );        

        //if ( isSrcUart && !loginComplete ) UnlockUartLog();
    }

    // ------------------------------------------------------------------------------------

    void BootServer::Logout( Utils::SafeRef<ISbpConnection> src, bool withSave )
    {        
        *m_log << "Logout command received from " << src->Name() << EndRecord;
        if ( m_active.IsEmpty() || m_active != src ) return ProcessProtocolError(src, "Not active src try logout.");

        ClientRespCmd::Send(src->Proto(), BfBootCore::Ok);

        Disconnect(withSave); 
    }

    // ------------------------------------------------------------------------------------

    void BootServer::RunAditionalScript( Utils::SafeRef<ISbpConnection> src, const std::string& scriptName, bool withSaveChanges )
    {
        *m_log << "RunAditionalScript command received from " << src->Name() << EndRecord;
        if ( m_active.IsEmpty() || m_active != src ) return ProcessProtocolError(src, "Not active src try run script.");

        BfBootCore::UserResponseCode response =
            (m_params.Cfg.PrepareScript(scriptName)) ? BfBootCore::Ok : BfBootCore::errCantRunScript;
        
        ClientRespCmd::Send(src->Proto(), response);

        Disconnect(withSaveChanges); 
    }

    // ------------------------------------------------------------------------------------

    void BootServer::Disconnect(bool saveChanges)
    {
        *m_log << "Active client disconnected" << EndRecord;               

        m_bootControl.Disconnect(m_active, saveChanges); // приведет к перезагрузке либо получим обратно управление    
        if (m_broadcast) m_broadcast->UpdateOwnerInfo( "" );
                
        //if (m_activeBroadcast) m_activeBroadcast->Start();
        ISbpConnection* connection = m_active.Clear();
        
        if (connection != m_uartConnection.get()) connection->UnbindUser();
    }

    // ------------------------------------------------------------------------------------

    void BootServer::ConnectionDeactivated( Utils::SafeRef<ISbpConnection> src, const std::string &errInfo )
    {
        *m_log << "Connection deactivated. Src: " << src->Name() << "Error: " << errInfo <<  EndRecord;
        
        if (m_active == src) Disconnect();
    }

    // ------------------------------------------------------------------------------------

    void BootServer::ResponseReceived( Utils::SafeRef<ISbpConnection> src, shared_ptr<SbpRecvPack> data )
    {        
        ProcessProtocolError(src, "NotSupported");
    }

    // ------------------------------------------------------------------------------------

    void BootServer::InfoReceived( Utils::SafeRef<ISbpConnection> src, shared_ptr<SbpRecvPack> data )
    {
        ProcessProtocolError(src, "NotSupported");
    }           

    void BootServer::OnProcess( iCore::MsgTimer* pTimer )
    {      
        ESS_ASSERT(pTimer == &m_prcTimer);
        if (m_tcpSbp) m_tcpSbp->Process();            
        if (m_uartConnection) m_uartConnection->Process();
        
        m_connectionList.Process();
        
        if (m_bootControl.NextCmdTimeout())
        {
            Disconnect();
            // todo send InfoError
        }

        if (m_params.pKeyAbort && m_params.pKeyAbort->IsTaskOver())
        {
            m_params.Action.Reset();            
            if (!m_active.IsEmpty())
            {
                m_active.Clear();
                m_bootControl.Clear();
            }
            AsyncExit();
            //OnExit();
        }         
    }

    void BootServer::AsyncExit()
    {
        if (m_uartConnection != 0) m_uartConnection->UnbindUser();
        PutMsg(this, &BootServer::OnExit);
    }

    void BootServer::OnStartNetwork( iCore::MsgTimer* pTimer )
    {
        pTimer->Stop(); 
        if (!TryStartNetwork()) pTimer->Start(CTryStartNetMsec);
    }           
   
} // namespace BfBootSrv

