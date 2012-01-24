
#include "stdafx.h"

#include "Utils/HostInf.h"
#include "Utils/QtHelpers.h"
#include "Utils/AtomicTypes.h"
#include "Utils/IStoragable.h"
#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"
#include "iCore/ThreadRunner.h"
#include "iCore/MsgTimer.h"

#include "Telnet/TelnetClient.h"
#include "Telnet/TelnetServerSession.h"
#include "Telnet/TelnetHelpers.h"
#include "Telnet/TelnetCommands.h"
#include "Telnet/ITelnet.h"
#include "Telnet/TelnetServer.h"

#include "TelnetTests.h"
#include "TestString.h"

namespace 
{
    using namespace std;
    using namespace Telnet;
    using namespace TelnetTests;
    using namespace iCore;

    const Utils::HostInf CServerHost = Utils::HostInf("127.0.0.1", 50089);
    const string CCodecName = "KOI8-R";
    const bool CRepeatable = true;

    // -------------------------------------

    enum
    {
        CStepDuration = 1000,  /* ms */
        CSteps = 12,
        CTestTimeout = CStepDuration * CSteps,

        CTimerInterval = 50,

        CServerRunTimeout = 30000,  /* ms */
    };

    // -------------------------------------

    enum States
    {
        StartSt,            // 0 - default
        DisconnectedSt,     // 1
        EstablishedSt,      // 2
        SendUserDataSt,     // 3
        SendProtoDataSt,    // 4
        AsciiModeSt,        // 5
        BinaryModeSt,       // 6
        UnicodeModeSt,      // 7
        FinishSt,           // 8
    };

    // -------------------------------------

    States StateBySendingMode(SendingMode mode)
    {
	States res;
        switch (mode)
        {
        case (AsciiMode): 
            res = AsciiModeSt;
            break;

        case (BinaryMode):
            res = BinaryModeSt;
            break;

        case (UnicodeMode):
            res = UnicodeModeSt;
            break;

        default:
            TUT_ASSERT(0 && "Unknown sending mode!");
        }

        return res;
    }

    // -------------------------------------
    // class unify parametes used by test-classes 
    class TestParams // : public Utils::IStoragable
    {
        States m_svrState;
        States m_cltState;

        // QString m_svrLog;
        // QString m_cltLog;
        ITelnetTestLogger &m_log;

    /*
    private:
        void Load(QTextStream &stream) // override
        {
            TUT_ASSERT(0 && "Not implemented");
        }
        
        void Save(QTextStream &stream) const // override
        { 
            stream << m_log.Get(); // m_svrLog << m_cltLog;
        } 
    */
    public:
        TestParams(ITelnetTestLogger &log)
        : m_svrState(StartSt), m_cltState(StartSt), m_log(log)
        {}

        void setServerState(States state) { m_svrState = state; }
        void setClientState(States state) { m_cltState = state; }

        States getServerState() { return m_svrState; }
        States getClientState() { return m_cltState; }

        void AddServerLog(const QString &data) 
        { 
            m_log.Add("Server: ");
            m_log.Add(data, true);
        }

        void AddClientLog(const QString &data) 
        { 
            m_log.Add("Client: ");
            m_log.Add(data, true);
        }
    };

    // ------------------------------------------
    // TelentClient user sample class
    class TelnetClientOwner : 
        public MsgObject,
        public ITelnetClientEvents
    {
        typedef TelnetClientOwner T;
        TestParams &m_params;

        TelnetClient m_client;
        QString m_data;
        Utils::AtomicBool &m_done;
        int m_establishId;
        std::vector<SendingMode> m_changeModeOrder;
        size_t m_nextModeId;
        iCore::MsgTimer m_timer; // used for waiting for proto-reply 
        bool m_receiveProtoTestData;

        void ChangeState(States state)
        {
            // std::cout << "client state: " << state << std::endl; 
            m_params.setClientState(state);
        }

        States GetState()
        {
            return m_params.getClientState();
        }

        void ClearData() 
        {
            m_client.getHistory().ClearRaw();
            m_data.clear();
        }
        
    // implement ITelnetClientEvents
    private:
        void TelnetDataInd(const QString &data) 
        { 
            // m_params.AddClientLog("Telnet data indication");
            m_data += data;

            if (m_data == TestData::ServerShellData())
            {
                ClearData();

                // step 9
                m_params.AddClientLog("Step 9: Disconnect request");
                m_client.DisconnectReq();
            }

            if (m_data == TestData::ServerShellOutputData())
            {
                m_data.clear();

                m_receiveProtoTestData = true;

                // m_timer.Start(CTimerInterval, CRepeatable);
            }

            if (m_data == TestData::DataCheckSuccessful())
            {
                TUT_ASSERT(m_receiveProtoTestData);

                // finish test
                m_params.AddClientLog("Finish test");
                ChangeState(FinishSt);
                AsyncSetReadyForDeleteFlag(m_done);
            }
        }

        void TelnetDiscInd()
        {
            m_params.AddClientLog("Disconnect indication");

            ChangeState(DisconnectedSt);

            // step 7, 10
            m_params.AddClientLog("Step 7/10: Establish request");
            m_client.EstablishReq(CServerHost);
        }

        void TelnetSocketErrorInd(boost::shared_ptr<iNet::SocketError> error) 
        {
            TUT_ASSERT(0 && "TelnetSocketErrorInd");
        }

        void TelnetEstablishInd() 
        {
            m_params.AddClientLog("Establish indication");
            // end of step 0
            TUT_ASSERT((GetState() == StartSt) || (GetState() == DisconnectedSt));

            if (GetState() == DisconnectedSt) ++m_establishId;

            ChangeState(EstablishedSt);
            ClearData();

            switch (m_establishId)
            {
            case (0): 
                // step 1
                m_params.AddClientLog("Step 1: Change mode ");
                ChangeMode(m_changeModeOrder.at(0), CCodecName);
                break;

            case (1):
                // do nothing
                break;

            case (2):
                // step 11
                m_params.AddClientLog("Step 11: Send data");
                SendProtoTestData();
                m_timer.Start(CTimerInterval, CRepeatable);

            default:
                // do nothing
                break;
            }
        }

        void TelnetChangeModeInd(SendingMode mode)
        { 
            // m_params.AddClientLog("Change mode indication");
            // if mode changed by sending command
            if ((GetState() == SendProtoDataSt)
                || (GetState() == FinishSt)
                || (m_establishId != 0)) return;
            
            ChangeState(StateBySendingMode(mode));
            
            TUT_ASSERT((m_params.getServerState() == GetState())
                       && "Step 1 - 4" );

            // steps 2 - 4
            if (m_nextModeId < (m_changeModeOrder.size() - 1))
            {
                m_params.AddClientLog("Step 2/3/4: Change mode");

                ++m_nextModeId;
                ChangeMode(m_changeModeOrder.at(m_nextModeId));
                return;
            }

            // step 5
            m_params.AddClientLog("Step 5: Send data");
            ClearData();
            SendTestData();
        }

    private:
        void SendTestData()
        {
            ChangeState(SendUserDataSt);
            m_client.Send(TestData::ClientUserData());
        }

        void SendProtoTestData()
        {
            TUT_ASSERT(m_client.getHistory().IsEmptyRaw());
            ChangeState(SendProtoDataSt);

            m_client.Send(CTextBegin);
            // m_client.SendCmd(CMD_WILL, OPT_NAWS);    /* request init opt */
            m_client.SendWinSize(CWinWidth, CWinHeight);  /* request init opt */
            m_client.SendCmd(CMD_WILL, OPT_TSPEED);     /* non-excist opt */
            m_client.Send(CTextMid);
            m_client.SendCmd(CMD_DO, OPT_SGA);          /* enabled opt */
            m_client.SendCmd(CMD_DONT, OPT_SGA);
            m_client.SendCmd(CMD_DO, OPT_SGA);          /* disabled opt */
            m_client.SendXDisplayLocation(CLocation);
            m_client.SendTerminalType(CTerminalType);
            m_client.Send(CTextEnd);
        }

        void SendDataCheckSuccessful()
        {
            m_client.Send(TestData::DataCheckSuccessful());
        }

        void ChangeMode(SendingMode mode, const string &codecName = "") 
        {
            // change state AFTER received indication
            m_client.ChangeSendingModeReq(mode, codecName);
        }

        // timer event
        void OnWaitServerReply(iCore::MsgTimer *pT)
        {
            // wait for receiving telnet data
            if (m_client.getHistory().getRawDataInput() 
                != TestData::ServerSocketOutputData()) return;

            pT->Stop();

            // step 12
            m_params.AddClientLog("Step 12: Data check successfull");
            SendDataCheckSuccessful();
        }

        QString TelnetInnerLog()
        {
            typedef TelnetDataLogger TL;
            return m_client.getHistory().getParsedDataAll(TL::AllData);
        }

    public:

        // ESS_TYPEDEF(SocketErrorInfo);

        TelnetClientOwner(IThreadRunner &runner, TestParams &params)
        : MsgObject(runner.getThread()), m_params(params),
          m_client(runner.getThread(), this, TelnetClient::TelnetInitParams(), true),
          m_done(runner.getCompletedFlag()), m_establishId(0),
          m_nextModeId(0), m_timer(this, &T::OnWaitServerReply), 
          m_receiveProtoTestData(false)
        {
            m_changeModeOrder.push_back(AsciiMode); // with codec
            m_changeModeOrder.push_back(BinaryMode);
            m_changeModeOrder.push_back(UnicodeMode);
            m_changeModeOrder.push_back(AsciiMode);

            m_client.EstablishReq(CServerHost);
        }

        ~TelnetClientOwner()
        {
            m_params.AddClientLog("Client closed.");
            m_params.AddClientLog("Inner telnet session log:");
            m_params.AddClientLog(TelnetInnerLog());
        }
    };

    // -------------------------------------
    // TelnetServer user sample class
    class TelnetServerOwner : 
        public MsgObject,
        public ITelnetServerSessionEvents,  
        public ITelnetServerEvents
    {
        typedef TelnetServerOwner T;

        MsgThread &m_thread;
        TestParams &m_params;

        TelnetServer m_server;
        shared_ptr<TelnetServerSession> m_pSession;
        QString m_data;
        int m_establishId;
        iCore::MsgTimer m_timer;
        bool m_receiveProtoTestData;

        void ChangeState(States state)
        {
            // std::cout << "server state: " << state << std::endl;
            m_params.setServerState(state);
        }

        States GetState()
        {
            return m_params.getServerState();
        }

        void ClearData() 
        {
            m_pSession->getHistory().ClearRaw();
            m_data.clear();
        }

    // implement ITelnetServerSessionEvents
    private:
        void TelnetDataInd(const QString &data) 
        {
            m_params.AddServerLog("Data indication");
            m_data += data;

            m_params.AddServerLog(data);
            m_params.AddServerLog(m_data);
            
            // end of step 5
            if (m_data == TestData::ClientUserData())
            {
                ClearData();
                
                // step 6
                m_params.AddServerLog("Step 6: Disconnect request");
                m_pSession->DisconnectReq();
            }

            if (m_data == TestData::ClientUserOutputData())
            {
                m_data.clear();
                m_receiveProtoTestData = true;
            }

            if (m_data == TestData::DataCheckSuccessful())
            {
                ClearData();

                TUT_ASSERT(m_receiveProtoTestData);

                // step 13
                m_params.AddServerLog("Step 13: Send data");
                SendProtoTestData();
                m_timer.Start(CTimerInterval, CRepeatable);
            }
        }

        void TelnetDiscInd() 
        {
            m_params.AddServerLog("Disconnect indication");
            //m_params.AddServerLog("Inner telnet session log");
            //m_params.AddServerLog(m_pSession->getHistory().getParsedDataAll(TelnetDataLogger::AllData));
            
            ChangeState(DisconnectedSt);

            // wrap in msg because we cannot delete owner from his own msg
            PutMsg(this, &TelnetServerOwner::OnDisconnectInd);
        }

        void TelnetEstablishInd() 
        {
            m_params.AddServerLog("Establish indication");
            TUT_ASSERT((GetState() == StartSt) || (GetState() == DisconnectedSt));

            if (GetState() == DisconnectedSt) ++m_establishId;

            ChangeState(EstablishedSt);
            ClearData();

            // step 8
            if (m_establishId == 1)
            {
                m_params.AddServerLog("Step 8: Send data");
                SendTestData();
            }
        }

        void TelnetChangeWinSizeInd(word width, word height)
        {
            // m_params.AddServerLog("WinSize indication");
            // test right parsing NAWS-string

            if (GetState() != SendProtoDataSt) return;

            TUT_ASSERT(width == CWinWidth);
            TUT_ASSERT(height == CWinHeight);
        }

        void TelnetChangeSendingModeInd(SendingMode mode)
        {
            // m_params.AddServerLog("Change sending mode indication");
            // don't change state on step 9
            if ((GetState() == SendProtoDataSt) || (m_establishId != 0))
            {
                
                return;
            }

            ChangeState(StateBySendingMode(mode));
        }

        void TelnetSocketErrorInd(boost::shared_ptr<iNet::SocketError> error) 
        {
            TUT_ASSERT(0 && "TelnetSocketErrorInd");
        }

    private:

        void OnDisconnectInd()
        {
            m_pSession.reset();
        }

        void SendProtoTestData()
        {
            TUT_ASSERT(m_pSession.get() != 0);
            
            ChangeState(SendProtoDataSt);
            
            m_pSession->Send(CTextBegin);
            m_pSession->SendCmd(CMD_DO, OPT_TRANSMIT_BINARY);       /* disabled opt */
            m_pSession->SendCmd(CMD_DONT, OPT_TRANSMIT_BINARY);
            m_pSession->Send(CTextMid);
            m_pSession->SendCmd(CMD_WONT, OPT_NEW_ENVIRON);
            m_pSession->SendCmd(CMD_DO, OPT_SGA);                   /* enabled opt */
            m_pSession->SendCmd(CMD_DO, OPT_NAWS);                  /* request-init opt */
            m_pSession->SendCmd(CMD_SB, OPT_NAWS);
            m_pSession->SendCmd(CMD_DO, OPT_TSPEED);                /* non-exist opt */
            m_pSession->Send(CTextEnd);
        }

        void SendTestData()
        {
            TUT_ASSERT(GetState() != DisconnectedSt);

            m_pSession->Send(TestData::ServerShellData());
        }

        void SendDataCheckSuccessful()
        {
            m_pSession->Send(TestData::DataCheckSuccessful());
        }

        // timer event
        void OnWaitClientReply(iCore::MsgTimer *pT)
        {
            // wait for receiving telnet data
            if (m_pSession->getHistory().getRawDataInput() 
                != TestData::ClientSocketOutputData()) return;

            pT->Stop();

            // step 14 (finish test)
            m_params.AddServerLog("Step 14: Finish test request");
            ChangeState(FinishSt);
            SendDataCheckSuccessful();
        }

    // implement ITelnetServerEvents
    public:
        void ServerNewClientConnectInd(boost::shared_ptr<iNet::ITcpSocket> socket)
        {
            m_params.AddServerLog("New client connected");
            TUT_ASSERT(socket.get() != 0);
            TUT_ASSERT(m_pSession.get() == 0);

            m_pSession = shared_ptr<TelnetServerSession>(
                    new TelnetServerSession(m_thread, socket, this, true));
        }

        void ServerSocketErrorInd(boost::shared_ptr<iNet::SocketError> error)
        {
            std::string info = error->getErrorString().toStdString();
            m_params.AddServerLog(QString("ServerSocket error indication: %1").arg(info.c_str()));
            ESS_THROW_MSG(SocketErrorInfo, info);
        }

        QString TelnetInnerLog()
        {
            TUT_ASSERT(m_pSession.get() != 0);
           
            return m_pSession->getHistory().getParsedDataAll(TelnetDataLogger::AllData); 
        }
        
    public:

        ESS_TYPEDEF(SocketErrorInfo);

        TelnetServerOwner(IThreadRunner &runner, TestParams &params)
        : MsgObject(runner.getThread()), m_thread(runner.getThread()),
          m_params(params),
          m_server(runner.getThread(), this, CServerHost),
          m_establishId(0), m_timer(this, &T::OnWaitClientReply),
          m_receiveProtoTestData(false)
        {            
        }

        ~TelnetServerOwner()
        {
            if (m_pSession.get() == 0) return;

            m_params.AddServerLog("Server closed.");
            m_params.AddServerLog("Inner telnet session log:");
            m_params.AddServerLog(TelnetInnerLog());
        }
    };

    // ------------------------------------------
    // telnet system test class
    class TelnetClientServerTestClass
    {
        TelnetServerOwner m_server;
        TelnetClientOwner m_client;
        TestParams &m_params;

    public:
        TelnetClientServerTestClass(IThreadRunner &runner, TestParams &params)
        : m_server(runner, params), m_client(runner, params), m_params(params)
        {
        }

        ~TelnetClientServerTestClass()
        {}
    };

} // namespace 

// -----------------------------------------

namespace TelnetTests
{
    void TelnetClientServerTest(ITelnetTestLogger &testLog)
    {
        testLog.Add("TelnetClientServerTest ... ", true);
        
        TestParams params(testLog);
        iCore::ThreadRunner test(Platform::Thread::LowPriority, CTestTimeout);

        // client-server test
        bool res = test.Run<TelnetClientServerTestClass>("TelnetClientServerTestClass", params);

        // if(!testLog.IsSilentMode()) params.SaveToFile("Telnet_proto_test_log.txt");
        
        if (res)
        {
            TUT_ASSERT((params.getClientState() == FinishSt) 
                       && (params.getServerState() == FinishSt));

            testLog.Add("OK!", true);
            return;
        }

        TUT_ASSERT(0 && "Test timeout!");
    }

    // --------------------------------------------

    void RunTelnetServer(ITelnetTestLogger &testLog)
    {
        cout << endl << "Test telnet server is activated for 30 sec" << endl; 

        TestParams params(testLog);

        iCore::ThreadRunner thread(Platform::Thread::LowPriority, CServerRunTimeout);

        thread.Run<TelnetServerOwner>("TelnetServerOwner", params);

        // params.SaveToFile("Telnet_test_server_run_log.txt");
    }

    // --------------------------------------------

    void RunTelnetClient()
    {
        // ...
    }


} // namespace TelnetTests
