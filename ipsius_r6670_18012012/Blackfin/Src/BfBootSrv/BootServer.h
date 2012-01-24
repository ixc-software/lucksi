#ifndef BOOTSERVER_H
#define BOOTSERVER_H

#include "iCore/ThreadRunner.h"
#include "SafeBiProtoExt/SpbTcpServer.h"
#include "BootControl.h"
#include "iLog/LogManager.h"
#include "BroadcastAutoSender.h"
#include "SafeBiProtoExt/SbpConnectionList.h"
#include "ExitAction.h"
#include "Utils/IExitTaskObserver.h"
#include "IReload.h"


namespace BfBootSrv
{
    using boost::shared_ptr;
    using boost::scoped_ptr;
    using SBProtoExt::ISbpConnection;
    using SBProtoExt::SbpConnection;
    using SBProto::SbpRecvPack;
    using SBProtoExt::SbpTcpServer;      

    // ------------------------------------------------------------------------------------

    // упаковка параметров конструктора сервера
    struct BootServerParams 
        : boost::noncopyable 
    {   
        enum {Unused = -1};

        BoardSetup& Cfg;
        IReload& Reload;        
        ExitAction Action;
        
        SBProto::SbpSettings SbpProf;
        
        iLogW::LogSettings LogSettings;
        bool CreteTraceActive; // создавать сесии включенными

        std::string LogPrefix;
        int CbpPort;        

        int COM;
        int BroadcastSrcPort;
        Utils::IExitTaskObserver* pKeyAbort;

        bool IsValid() const
        {
            return                        
                CbpPort <= 65535 &&
                BroadcastSrcPort >= 0;
        }


        BootServerParams(BoardSetup& cfg, IReload& reload, const iLogW::LogSettings& logSettings)
            : Cfg(cfg),                        
            Reload(reload),            
            Action(reload),
            LogSettings(logSettings),
            CreteTraceActive(false),
            CbpPort(0),           
            COM(Unused),
            BroadcastSrcPort(-1),
            pKeyAbort(0)
        {}
    };        

    // ------------------------------------------------------------------------------------    

    class BootServer : 
        public boost::noncopyable,
        public iCore::MsgObject,                
        public SBProtoExt::ISbpConnectionEvents, 
        public SBProtoExt::ISbpTcpServerEvents,         
        public Utils::IVirtualDestroyOwner, // для uartConection
        public IBootCtrlOwner
    {                        
        struct CheckParams {  CheckParams(const BootServerParams& params);   };
        
        //ServerErrHook m_errInformer; // todo
        CheckParams m_paramsOk;             
        BootServerParams& m_params;
        iCore::IThreadRunner& m_runner;
                
        boost::shared_ptr<SBProtoExt::SbpLogSettings> m_sbpLogParam;
        iLogW::LogManager m_logMng;
        scoped_ptr<iLogW::LogSession> m_log;
        iLogW::LogRecordTag m_tagObjInfo; // информация о сервере
        std::string m_serverInfo;  // используется если в стартовых параметрах трассировка выключенна
       
        scoped_ptr<BroadcastAutoSender> m_broadcast;
        scoped_ptr<SbpTcpServer> m_tcpSbp;
                
        scoped_ptr<ISbpConnection> m_uartConnection; //всегда активно. Удаление через scoped_ptr или через Unbind?
        
        BootControl m_bootControl; // объект посредством которого удаленный клиент ведет управление            
        
        Utils::SafeRef<ISbpConnection> m_active;
        iCore::MsgTimer m_prcTimer;

        SBProtoExt::SbpConnectionList m_connectionList;                                        

        iCore::MsgTimer m_startNetTimer;

        //BfDev::BfWatchdog m_watchDog;

    // IVirtualDestroyOwner
    private:
        void Add(Utils::IVirtualDestroy*)
        {/*nothing*/}
        void Delete(Utils::IVirtualDestroy*)
        {ESS_HALT("Newer call");}

    // own
    private:                               
        void Disconnect(bool saveChanges = false);                
        void ProcessProtocolError(Utils::SafeRef<ISbpConnection> src, const std::string& errTxt);
        void OnProcess(iCore::MsgTimer* pTimer);
        void OnExit()
        {          
            *m_log << "BootServer work finished." << iLogW::EndRecord;
            m_runner.getCompletedFlag().Set(true);
        }
        void OnUartRestart();

        void InitUart();
        void InitNetworkTools();
        void OnStartNetwork(iCore::MsgTimer* pTimer);
        bool TryStartNetwork();

//         void OnNextCmdTimeout(iCore::MsgTimer* p);
//         void OnWaitNextCmd();

    // ISbpTcpServerEvents 
    private:
        void NewConnection(const SbpTcpServer*, boost::shared_ptr<ISbpConnection>);
        void Error(const SbpTcpServer *pSrv, const std::string &)
        {
            ESS_ASSERT(pSrv == m_tcpSbp.get());
            if (!m_active.IsEmpty()) Disconnect();
            // todo если нет активного соединения надо ждать когда BootCtrl попробует 
            ESS_HALT("SbpTcpServer error occurred when server can`t reload.");
        }

    // ISbpConnectionEvents
    private:
        void CommandReceived(Utils::SafeRef<ISbpConnection> src,  shared_ptr<SbpRecvPack> data);
        void ResponseReceived(Utils::SafeRef<ISbpConnection> src, shared_ptr<SbpRecvPack> data);
        void InfoReceived(Utils::SafeRef<ISbpConnection> src, shared_ptr<SbpRecvPack> data);
        void ProtocolError(Utils::SafeRef<ISbpConnection> src, boost::shared_ptr<SBProto::SbpError> err);

        void ConnectionActivated(Utils::SafeRef<ISbpConnection>) 
        { 
            //ESS_HALT("!Break");            
        }        
        void ConnectionDeactivated(Utils::SafeRef<ISbpConnection> src,  const std::string &errInfo);        
    
    private:
        class LoginAdapter;

        void Login(Utils::SafeRef<ISbpConnection> src, const std::string& pwd, dword protoVersion);
        void Logout(Utils::SafeRef<ISbpConnection> src, bool withSave);
        void RunAditionalScript(Utils::SafeRef<ISbpConnection> src, const std::string& scriptName, bool withSaveChanges);
        void SetTrace(Utils::SafeRef<ISbpConnection> src, bool on);

        //void SetTrace(bool on);

    // IBootCtrlOwner impl
    private:
        void AsyncExit();         

    public:       
        BootServer( iCore::IThreadRunner& runner, BootServerParams& params );
        ~BootServer();

    };
} // namespace BfBootSrv

#endif
