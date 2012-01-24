
#include "stdafx.h"
#include "SafeBiProtoTestBase.h"
#include "SafeBiProtoTests.h"

#include "Utils/DebugAllocationCounter.h"


namespace 
{
    using namespace SBPTests;

    const std::string CClientString = std::string(225, 'c'); //"client side info";
    const std::string CServerString = std::string(225, 's');// "server side info";

    // ------------------------------------------------------
    
    class TestParams: 
        public SafeBiProtoTestParams
    {
        word m_packCount;
        byte m_iterations;
        
    public:
        TestParams(ITestTransportsCreator &creator, bool testWithMsgs, word packCount, 
                   byte iterations) : 
            SafeBiProtoTestParams(creator, testWithMsgs, false, true), m_packCount(packCount),
            m_iterations(iterations)
        {
        }

        word PackCount() const { return m_packCount; }
        byte Iterations() const { return  m_iterations; }
    };
        
    // ------------------------------------------------------
    
    class Client : public ClientSideBase
    {
        Utils::AllocCounter m_allocCounter;
        TestParams m_params;
        word m_recvCounter;
        byte m_iterCounter;
        MiniLogger m_log;
        Utils::AtomicBool &m_done;

        void LogAllocatedBlocks()
        {
            TUT_ASSERT(m_allocCounter.ModeEnabled());
            
            m_log.Add("Blocks allocated -- ", m_allocCounter.Get());
        }

        void SendPack()
        {
            SbpSendPackInfo pack(Protocol());
            pack.WriteString(CClientString); 
        }

        void SendNPacks()
        {
            m_log.Add("Iteration #", m_iterCounter  + 1);
            m_log << "Sending start...";
            for (int i = 0; i < m_params.PackCount(); ++i)
            {
                SendPack();
            }
            ++m_iterCounter;
        }

    private:
        void OnInfoReceived(boost::shared_ptr<SbpRecvPack> data) // override
        {
            TUT_ASSERT(m_iterCounter <= m_params.Iterations());
            TUT_ASSERT(data.get() != 0);
            TUT_ASSERT(data->Curr().AsString() == CServerString);
            ++m_recvCounter;

            if (m_recvCounter < m_params.PackCount()) return;
            
            m_log.Add("Packets received -- ", m_recvCounter);
            LogAllocatedBlocks();
            
            m_recvCounter = 0;
            
            if (m_iterCounter < m_params.Iterations()) SendNPacks();
            else AsyncSetReadyForDeleteFlag(m_done);
        }
        
        void OnConnected() // override
        {
            m_log << "Start";
            LogAllocatedBlocks();
            
            SendNPacks();
        }
        
    public:
        Client(iCore::IThreadRunner &runner, TestParams &params) : 
            ClientSideBase(runner, params), m_params(params),
            m_recvCounter(0), m_iterCounter(0), m_log("Client", false), 
            m_done(runner.getCompletedFlag())
        {
        }
    };

    // ------------------------------------------------------

    class Server : public ServerSideBase
    {
        Utils::AllocCounter m_allocCounter;
        TestParams m_params;
        word m_recvCounter;
        byte m_iterCounter;
        MiniLogger m_log;
        
        void LogAllocatedBlocks()
        {
            TUT_ASSERT(m_allocCounter.ModeEnabled());
            
            m_log.Add("Blocks allocated -- ", m_allocCounter.Get());
        }
        
        void SendPack()
        {
            SbpSendPackInfo pack(Protocol());
            pack.WriteString(CServerString); 
        }

        void SendNPacks()
        {
            m_log.Add("Iteration #", m_iterCounter  + 1);
            m_log << "Sending start...";
            for (int i = 0; i < m_params.PackCount(); ++i)
            {
                SendPack();
            }
            ++m_iterCounter;
        }

    private:
        void OnInfoReceived(boost::shared_ptr<SbpRecvPack> data) // override
        {
            TUT_ASSERT(m_iterCounter <= m_params.Iterations());
            TUT_ASSERT(data.get() != 0);
            TUT_ASSERT(data->Curr().AsString() == CClientString);
            ++m_recvCounter;
            
            if (m_recvCounter < m_params.PackCount()) return;
            
            m_log.Add("Packets received -- ", m_recvCounter);
            LogAllocatedBlocks();
            
            m_recvCounter = 0;

            if (m_iterCounter < m_params.Iterations()) SendNPacks();
        }
        
        void OnNewTransport() // override
        {
            m_log << "Start";
            LogAllocatedBlocks();
        }

        void OnSentPacketInfo(const SbpPackInfo &data, bool isSentPack) // override
        {
            // do nothing
        }
        
    public:
        Server(iCore::IThreadRunner &runner, TestParams &params) : 
            ServerSideBase(runner, params), m_params(params),
            m_recvCounter(0), m_iterCounter(0), m_log("Server", false)
        {
        }
    };

    // ------------------------------------------------------

    class PackExchangeClass : public iCore::MsgObject
    {
        Server m_server;
        Client m_client;
        
    public:
        PackExchangeClass(iCore::IThreadRunner &runner, TestParams &params)
        : iCore::MsgObject(runner.getThread()), 
            m_server(runner, params), m_client(runner, params)
        {
        }
    };
    
} // namespace 

// ------------------------------------------------------

namespace SBPTests
{
    void SbpPackExchange(ITestTransportsCreator &creator, bool testWithMsgs, 
                         word packCount, byte iterations, dword timeoutMs)
    {
        int before = Utils::AllocCounter().Get();
        
        TestParams params(creator, testWithMsgs, packCount, iterations);

        iCore::ThreadRunner runner(Platform::Thread::LowPriority, timeoutMs);
        
        if (!runner.Run<PackExchangeClass>(params)) TUT_ASSERT(0 && "SbpPackExchange timeout");

        int after = Utils::AllocCounter().Get();
        std::cout << "AllocCounter -- " << before << "(+" << (after - before) << ")" << std::endl;
    }
    
} // namespace SBPTests
