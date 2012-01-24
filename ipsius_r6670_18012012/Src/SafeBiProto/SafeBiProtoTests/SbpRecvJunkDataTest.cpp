
#include "stdafx.h"
#include "SafeBiProtoTests.h"
#include "SafeBiProtoTestBase.h"
#include "TestTcpTransportCreator.h"
#include "Utils/Random.h"
#include "Platform/Platform.h" // for random seed
#include "iNet/TcpSocket.h"
#include "iNet/ITcpSocketToUser.h"
#include "SafeBiProto/SbpTypes.h"
#include "Utils/AtomicTypes.h"

namespace 
{
    using namespace SBPTests;
    using namespace SBProto;

    // const int CSentBytes = 1048576; // 1Mb
    // const int CSentPartCount = 1024;
    // const int CMaxSendPart = 1024;
    // const int CMinSendPart = 1;
    const int CSymbolRange = 5; 

    const dword CWaitLastErrIntervalMs = 100;

    /*
    ITestTransportsCreator &creator, bool testWithMsgs, 
                              bool silentMode, bool doNotProcessTimeouts = false
    */

    class TestParams: 
        public SafeBiProtoTestParams
    {
        dword m_sentPartsCount;
        dword m_maxSentPart;
        bool m_showErrLog;
        
    public:
        TestParams(ITestTransportsCreator &creator, bool silentMode, dword sentPartsCount,
                   dword maxSentPart, bool showErrLog) : 
            SafeBiProtoTestParams(creator, true, silentMode, true), 
            m_sentPartsCount(sentPartsCount), m_maxSentPart(maxSentPart),
            m_showErrLog(showErrLog)
        {
            TUT_ASSERT(maxSentPart > 0);
            TUT_ASSERT(sentPartsCount > 0);
        }

        dword SentPartsCount() const { return m_sentPartsCount; }
        dword MaxSentPart() const { return  m_maxSentPart; }
        bool ShowErrLog() const { return m_showErrLog; }
    };

    // --------------------------------------------------------------------------
    
    class ClientSide : public ClientSideBase
    {
        TestParams &m_params;
        
    private:
        void OnConnected() // override
        {
            Utils::Random rand(Platform::GetSystemTickCount());
            int summ = 0;
            // while (summ < CSentBytes)
            for (int i = 0; i < m_params.SentPartsCount(); ++i)
            {
                // send data 
                std::vector<byte> data;
                
                dword size = (m_params.MaxSentPart() == 1)? 
                              1 : rand.Next(m_params.MaxSentPart() - 1) + 1;
                
                for (dword i = 0; i < size; ++i)
                {
                    data.push_back(rand.Next(CSymbolRange));
                }
    
                Transport().DataSend(&data.at(0), data.size());
            
                summ += size;
                
                // byte data = 5;
                // Transport().Send(&data, sizeof(data));
            }
            
            if (m_params.getSilentMode()) return;
            
            std::cout << "Overall sent data size: " << summ << std::endl;
        }

    public:
        ClientSide(iCore::IThreadRunner &runner, TestParams &params) :
            ClientSideBase(runner, params), m_params(params)
        {
        }
    };

    // --------------------------------------------------------------------------

    class ServerSide : public ServerSideBase
    {
        Utils::AtomicBool &m_done;
        TestParams &m_params;
        // int m_counter;
        iCore::MsgTimer m_timer;

        void OnConnected() // override
        {
            // do nothing
        }
        
        void OnNewTransport() // override
        {
            // do nothing
        }

        void OnCommandReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            m_timer.Stop();
            Log("Command received(!)", data->ToString());
            m_timer.Start(CWaitLastErrIntervalMs); 
        }
        
        void OnInfoReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            m_timer.Stop();
            Log("Info received(!)", data->ToString());
            m_timer.Start(CWaitLastErrIntervalMs); 
        }
        
        void OnResponceReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            m_timer.Stop();
            Log("Responce received(!)", data->ToString());
            m_timer.Start(CWaitLastErrIntervalMs); 
        }
        
        void OnProtocolError(boost::shared_ptr<SbpError> err) // override
        {
            // ++m_counter;
            m_timer.Stop();

            Log("Protocol error", err->ToString());
            Protocol().Activate();

            m_timer.Start(CWaitLastErrIntervalMs); 
            
            // if (m_counter < m_params.SentPartsCount()) return;
            // AsyncSetReadyForDeleteFlag(m_done);
        }
        
        void OnSentPacketInfo(const SbpPackInfo &data, bool isSentPack) // override
        {
            // do nothing
        }

        void Log(const std::string &tag, const std::string &data)
        {
            if (m_params.ShowErrLog()) std::cout << tag << " : " << data << std::endl;
        }

        
        // timer event
        void OnWaitLastError(iCore::MsgTimer *pT)
        {
            AsyncSetReadyForDeleteFlag(m_done);
        }
        
    public:
        ServerSide(iCore::IThreadRunner &runner, TestParams &params) :
            ServerSideBase(runner, params), m_done(runner.getCompletedFlag()),
            m_params(params)/*, m_counter(0)*/, m_timer(this, &ServerSide::OnWaitLastError)
        {
            m_timer.Start(CWaitLastErrIntervalMs); 
        }
        
    };

    // --------------------------------------------------------------------------

    class SbpRecvJunkDataTestClass : public iCore::MsgObject
    {
        ServerSide m_server;
        ClientSide m_client;
        
    public:
        SbpRecvJunkDataTestClass(iCore::IThreadRunner &runner, TestParams &params)
        : iCore::MsgObject(runner.getThread()), 
            m_server(runner, params), m_client(runner, params)
        {
        }
    };
    
} // namespace

// --------------------------------------------------------------------------

namespace SBPTests
{
    void SbpRecvJunkDataTest(dword sentPartsCount, dword maxSentPart,
                             dword timeout, bool showErrLog, bool silentMode)
    {
        if (!silentMode) std::cout << "SbpRecvJunkData test start ... " << std::endl;
        
        TestTcpTransportsCreator creator;
        TestParams params(creator, silentMode, sentPartsCount, maxSentPart, showErrLog);
        iCore::ThreadRunner runner(Platform::Thread::LowPriority, timeout);
        
        if (runner.Run<SbpRecvJunkDataTestClass>(params))
        {
            if (!silentMode) std::cout << "SbpRecvJunkData test end." << std::endl;
            return;
        }

        TUT_ASSERT(0 && "SbpRecvJunkDataTest timeout");
    }
    
} // namespace SBPTests
