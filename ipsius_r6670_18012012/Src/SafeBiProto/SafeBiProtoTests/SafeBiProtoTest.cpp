
#include "stdafx.h"
#include "SafeBiProtoTests.h"

#include "SafeBiProtoTestBase.h"

namespace
{
    using namespace SBProto;
    using namespace SBPTests;
    
    const int CTestTimeoutMs = 15 * 1000;
    

    /*
        Test scheme:

            | OnConnected/ |OnDisc|  OnCommand  |OnResponce     |   OnInfo   |  Timeouts     |
            | NewTransport |      |             |               |            |  resp |  recv |
     ________________________________________________________________________________________|
            |1)Send cmd    |6) +  |2)Send resp  |3)Send cmd     |3) ----     |       |       |
     server |              |      |  Send info  | Send info     |            |       |       |
            |              |      |5) ----      |5)Disc: m_done |            |       |       |
     ________________________________________________________________________________________|
            |1)Send cmd    |6) +  |2)Send info  |3) -----       |3) -----    |7) +   |       |
     client |              |      |  Send resp  |               |4) -----    | m_done|       |
            |              |      |4) Send cmd  |               |            |       |       |
            |              |      |   Send resp |               |            | exit  |       |

    * 1 - 7 -- stages
    
    */
    
    // -----------------------------------------------------------

    /*
         Server stages variables:
    */
    
    /*const*/ std::string CServerStage1CmdName = "get_timeout";

    const bool CServerStage2Resp = false; // to client stage 1 cmd
    /*const*/ std::wstring CServerStage2Info = L"Server info";
    
    /*const*/ std::string CServerStage3CmdName = "reset_var";
    /*const*/ std::string CServerStage3Info = "Some user info";
    
    /* 
        Client stages variables:
    */
    
    /*const*/ std::string CClientStage1CmdName = "open_file";
    /*const*/ std::string CClientStage1CmdParam1 = "path/fileName.txt";
    const byte CClientStage1CmdParam2 = 1; // open mode

    /*const*/ std::string CClientStage2Info = "Client info";
    const dword CClientStage2Resp = 60000; // to server stage 1 cmd

    /*const*/ std::string CClientStage4CmdName = "set_flag";
    const bool CClientStage4CmdParam = true;
    /*const*/ std::string CClientStage4Recp = "ok"; // to server stage 3 cmd
    
    // -----------------------------------------------------------

    class ClientSide: public ClientSideBase
    {
        MiniLogger m_log;
        SafeBiProtoTestParams &m_params;

        int m_cmdCounter;
        int m_infoCounter;

        Utils::AtomicBool &m_done;

    private:
        void OnCommandReceived(boost::shared_ptr<SbpRecvPack> data) // override
        {
            TUT_ASSERT(data.get() != 0);
            switch (m_cmdCounter)
            {
            case 0:
                // Stage 2:
                m_log << "Stage 2: Command received, Info sent, Responce sent"; // << data->ToString();
                // Read server stage 1 data
                {
                    TUT_ASSERT(data->Curr().AsString() == CServerStage1CmdName);
                }
                {
                    SbpSendPackInfo info(Protocol());
                    info.WriteString(CClientStage2Info);
                }// send
                {
                    SbpSendPackResp resp(Protocol());
                    resp.WriteDword(CClientStage2Resp);
                } // send
                break;

            case 1:
                
                // Stage 4:
                m_log << "Stage 4: Command received, Command sent, Responce sent"; // << data->ToString();
                // Read server stage 3 data
                {
                    TUT_ASSERT(data->Curr().AsString() == CServerStage3CmdName);
                }
                {
                    SbpSendPackCmd cmd(Protocol());
                    cmd.WriteString(CClientStage4CmdName).WriteBool(CClientStage4CmdParam);
                } // send
                {
                    SbpSendPackResp resp(Protocol());
                    resp.WriteString(CClientStage4Recp);
                } // send
                
                break;
            
            default:
                TUT_ASSERT(0 && "Invalid client side command counter value");
            }

            ++m_cmdCounter;
        }

        void OnInfoReceived(boost::shared_ptr<SbpRecvPack> data) // override
        {
            TUT_ASSERT(data.get() != 0);
            
            switch (m_infoCounter)
            {
            case 0:
                // Stage 3: read server stage 2 info
                m_log << "Stage 3: Info received"; // << data->ToString();
                {
                    TUT_ASSERT(data->Curr().AsWstring() == CServerStage2Info);
                }
                break;
            
            case 1:
                // Stage 4: read server stage 3 info
                m_log << "Stage 4: Info Received"; // << data->ToString();
                {
                    TUT_ASSERT(data->Curr().AsString() == CServerStage3Info);
                }
                
                break;

            default:
                TUT_ASSERT(0 && "Invalid client side responce counter value");
            }
            
            ++m_infoCounter;
        }

        void OnResponseReceived(boost::shared_ptr<SbpRecvPack> data) // override
        {
            TUT_ASSERT(data.get() != 0);
            
            // Stage 3:
            m_log << "Stage 3: Responce received"; // << data->ToString();
            
            // Read server stage 2 data
            TUT_ASSERT(data->Curr().AsBool() == CServerStage2Resp);
        }
        
        void OnProtocolError(boost::shared_ptr<SbpError> err) // overrride
        {
            // Stage 6:

            if (typeid(*err.get()) 
                != typeid(SbpWaitingResponceTimeout)) ESS_HALT(err->ToString());

            TUT_ASSERT(m_cmdCounter == 2);
            TUT_ASSERT(m_infoCounter == 2);

            m_params.setClientFinished();

            m_log << "Stage 7: Responce timeout, Finished";
            
            AsyncSetReadyForDeleteFlag(m_done);
        }

        void OnConnected() // override
        {
            // Server has to first start action on OnNewTransport()
            Platform::Thread::Sleep(100); 

            m_log << "Stage 1: Conected, Command sent";

            // Stage 1:
            SbpSendPackCmd pack(Protocol());
            pack.WriteString(CClientStage1CmdName).WriteString(CClientStage1CmdParam1);
            pack.WriteByte(CClientStage1CmdParam2);
            
        } // send pack in ~SbpSendPackInfo()
        
        void OnDisconnected(const std::string &desc) // override
        {
            // Stage 6
            m_log << "Stage 6: Disconnected";
            
            TUT_ASSERT(m_cmdCounter == 2);
            TUT_ASSERT(m_infoCounter == 2);
            
            // TUT_ASSERT(0 && "Disconnected");
        }

    public:
        ClientSide(iCore::IThreadRunner &runner, SafeBiProtoTestParams &params)
        : ClientSideBase(runner, params),
            m_log("Client", params.getSilentMode()), m_params(params),
            m_cmdCounter(0), m_infoCounter(0), m_done(runner.getCompletedFlag())
        {
        }
    };

    // -----------------------------------------------------------

    // One-session server
    class ServerSide: public ServerSideBase
    {
        MiniLogger m_log;
        SafeBiProtoTestParams &m_params;

        int m_cmdCounter;
        int m_respCounter;
        
    private:
        void OnResponseReceived(boost::shared_ptr<SbpRecvPack> data) //override
        {
            TUT_ASSERT(data.get() != 0);
            
            switch (m_respCounter)
            {
            case 0:
                /*
                m_log << "Stage 3: Responce received"; // << data->ToString();
                // Read client stage 2 data
                {
                    TUT_ASSERT(data->Curr().AsDword() == CClientStage2Resp);
                }
                */
                m_log << "Stage 3: Responce received, Command sent, Info sent";
                // Read client stage 2 data
                TUT_ASSERT(data->Curr().AsDword() == CClientStage2Resp);
                {
                    SbpSendPackCmd cmd(Protocol());
                    cmd.WriteString(CServerStage3CmdName);
                } // send
                {
                    SbpSendPackInfo info(Protocol());
                    info.WriteString(CServerStage3Info);
                } // send
                
                break;

            case 1:
                m_log << "Stage 5: Responce received, Disconnect";
                // Read client stage 4 data 
                {
                    TUT_ASSERT(data->Curr().AsString() == CClientStage4Recp);
                }
                Transport().Disconnect();
                Protocol().Deactivate();
                break;
            
            default:
                TUT_ASSERT(0 && "Invalid server side responce counter value");
            }

            ++m_respCounter;
        }
        
        void OnInfoReceived(boost::shared_ptr<SbpRecvPack> data) // override 
        {
            TUT_ASSERT(data.get() != 0);
            
            // Stage 3
            /*
            m_log << "Stage 3: Info received, Command sent, Info sent";
            // Read client stage 2 data
            TUT_ASSERT(data->Curr().AsString() == CClientStage2Info);
            {
                SbpSendPackCmd cmd(Protocol());
                cmd.WriteString(CServerStage3CmdName);
            } // send
            {
                SbpSendPackInfo info(Protocol());
                info.WriteString(CServerStage3Info);
            } // send
            */

            m_log << "Stage 3: Info received"; // << data->ToString();
            // Read client stage 2 data
            {
                TUT_ASSERT(data->Curr().AsString() == CClientStage2Info);
            }
        }

        void OnCommandReceived(boost::shared_ptr<SbpRecvPack> data) // override
        {
            TUT_ASSERT(data.get() != 0);
            
            switch (m_cmdCounter)
            {
            case 0:
                // Stage 2:
                m_log << "Stage 2: Command received, Responce sent, Info sent"; // << data->ToString();
                {
                    // Read stage 1 data:
                    TUT_ASSERT(data->Curr().AsString() == CClientStage1CmdName);
                    data->Next();
                    TUT_ASSERT(data->Curr().AsString() == CClientStage1CmdParam1);
                    data->Next();
                    TUT_ASSERT(data->Curr().AsByte() == CClientStage1CmdParam2);
                    TUT_ASSERT(!data->Next());
                }
                {
                    SbpSendPackResp resp(Protocol());
                    resp.WriteBool(CServerStage2Resp);
                } // send
                {
                    SbpSendPackInfo info(Protocol());
                    info.WriteWstring(CServerStage2Info);
                } // send
                
                break;

            case 1:
                // Stage 5:
                m_log << "Stage 5: Command received, did not send responce";
                // Read client stage 4 data
                {
                     TUT_ASSERT(data->Curr().AsString() == CClientStage4CmdName);
                     data->Next();
                     TUT_ASSERT(data->Curr().AsBool() == CClientStage4CmdParam);
                }
                // do not send responce
                break;
            
            default:
                TUT_ASSERT(0 && "Invalid server side command counter value");
            }

            ++m_cmdCounter;
        }
    
        void OnProtocolError(boost::shared_ptr<SbpError> err) // override
        {
            ESS_HALT(err->ToString());
        }

        void OnNewTransport() // override
        {
            // Stage 1:
            m_log << "Stage 1: New connection, Command sent";
            SbpSendPackCmd cmd(Protocol());
            cmd.WriteString(CServerStage1CmdName);
            
        } // cmd sent
        
        void OnDisconnected(const std::string &desc) // override
        {
            // Stage 6:
            TUT_ASSERT(m_respCounter == 2);
            TUT_ASSERT(m_cmdCounter == 2);

            m_log << "Stage 6: Disconnected, Finished";
            m_params.setServerFinished();
            
            // TUT_ASSERT(0 && "Disconnected");
        }

        void OnSentPacketInfo(const SbpPackInfo &data, bool isSentPack)
        {
            // do nothing
        }
        
    public:
        ServerSide(iCore::IThreadRunner &runner, SafeBiProtoTestParams &params)
        : ServerSideBase(runner, params),
            m_log("Server", params.getSilentMode()),
            m_params(params), m_cmdCounter(0), m_respCounter(0)
        {
        }
    };
    
    // -----------------------------------------------------------
    
    class SafeBiProtoTestClass : public iCore::MsgObject
    {
        ServerSide m_server;
        ClientSide m_client;
        
    public:
        SafeBiProtoTestClass(iCore::IThreadRunner &runner, SafeBiProtoTestParams &params)
        : iCore::MsgObject(runner.getThread()), 
            m_server(runner, params), m_client(runner, params)
        {
        }
    };
    
    
} // namespace

// -----------------------------------------------------------

namespace SBPTests
{
    using namespace SBProto; 
    
    void SafeBiProtoTest(ITestTransportsCreator &creator, bool testWithMsgs, bool silentMode)
    {
        if (!silentMode) std::cout<< "\nSafeBiProtoTest start\n";

        SafeBiProtoTestParams params(creator, testWithMsgs, silentMode);
        iCore::ThreadRunner test(Platform::Thread::LowPriority, CTestTimeoutMs);
        
        if (test.Run<SafeBiProtoTestClass>("SafeBiProtoTestClass", params))
        {
            params.CheckFinished();

            if (!silentMode) std::cout << "SafeBiProto test okay." << std::endl;
            return;
        }
        
        TUT_ASSERT(0 && "SafeBiProto test timeout");
    }
       
} // namespace SBPTests 
    
