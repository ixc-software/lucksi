#ifndef TESTECHOCHIPCTRL_H
#define TESTECHOCHIPCTRL_H

#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "iLog/iLogSessionCreator.h"
#include "SafeBiProtoExt/ISbpConnection.h"
#include "SafeBiProto/ISbpTransport.h"
#include "SafeBiProto/SbpError.h"
#include "IEchoChipCtrlOwner.h"


namespace TestRealEcho
{
    using Utils::SafeRef;    
    using boost::shared_ptr;
    using boost::scoped_ptr;


    class EchoChipCtrl :
        public iCore::MsgObject,        
        public SBProtoExt::ISbpConnectionEvents,
        public EchoApp::IEchoRecvSide
    {
        scoped_ptr<SBProtoExt::ISbpConnection> m_connection;
        iCore::MsgTimer m_process;
        IEchoChipCtrlOwner& m_owner;             

    // ISbpConnectionEvents
    private:
        void CommandReceived(SafeRef<SBProtoExt::ISbpConnection> src,  shared_ptr<SBProto::SbpRecvPack> data);
        void ResponseReceived(SafeRef<SBProtoExt::ISbpConnection> src, shared_ptr<SBProto::SbpRecvPack> data);
        void InfoReceived(SafeRef<SBProtoExt::ISbpConnection> src, shared_ptr<SBProto::SbpRecvPack> data);
        void ProtocolError(SafeRef<SBProtoExt::ISbpConnection> src, boost::shared_ptr<SBProto::SbpError> err);
        void ConnectionActivated(SafeRef<SBProtoExt::ISbpConnection> src);
        void ConnectionDeactivated(SafeRef<SBProtoExt::ISbpConnection> src, const std::string & errInfo);

     // EchoApp::IEchoRecvSide impl
    private:
        void OnCmdInit(const EchoApp::CmdInitData &data); // response: OnEchoResponse
        void OnCmdEcho(int chNum, int taps);
        
        void OnCmdRequestState(); // OnEchoStateResp
        void OnCmdShutdown();                        
        void OnCmdRequestHalt();
		void OnCmdSetFreqRecvMode(int,const std::string &,const std::string &,bool);

    private:
        static shared_ptr<SBProto::ISbpTransport> CreateUart(int com, int boudRate);
        void OnErrorOccured(const std::string& err);
        void ErrorOccured(const std::string& err);
        void OnProcess(iCore::MsgTimer* pTimer);

    public:
        EchoChipCtrl(
            iCore::MsgThread& thread,
            IEchoChipCtrlOwner& owner,
            SafeRef<iLogW::ILogSessionCreator> log,
            int com, int boudRate
            );

        void SendUnknownCmd();
    };
} // namespace TestRealEcho

#endif
