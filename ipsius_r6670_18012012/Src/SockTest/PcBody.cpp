#include "stdafx.h"
#include "Domain/DomainTestRunner.h"
#include "Domain/DomainClass.h"
#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"
#include "Utils/ManagedList.h"
#include "Utils/IVirtualDestroy.h"
#include "PcBody.h"
#include "UdpTestSrvside.h"
#include "TcpSrvTestSrvside.h"
#include "TcpSockTestSrvside.h"
#include "ITestInfra.h"
#include "DataStreamTest.h"

using Domain::DomainClass;
using namespace SockTest;
using namespace iCore;

// ---------------------------------------------------------

namespace
{
    class RunParams {};

    class Test : 
        public MsgObject, 
        public ITestInfra
    {
        DomainClass &m_domain;
        Utils::ManagedList<Utils::IVirtualDestroy> m_list;
        MsgTimer m_timer;

        void OnTimer(MsgTimer *pT)
        {
            if (Platform::KeyWasPressed() > 0) m_domain.Stop();
        }

        void Log(const std::string &s)
        {
            std::cout << s << std::endl;
        }

    // ITestInfra impl
    private:

        void PrintToLog(const std::string &s)
        {
            Log(s);
        }


    public:

        Test(DomainClass &domain, RunParams &params) : 
          MsgObject(domain.getMsgThread()),
          m_domain(domain),
          m_timer(this, &Test::OnTimer)
        {            
            m_list.Add( new UdpTestSrvside(*this, domain.getMsgThread(), UdpTestSrvsideProfile::Create()) );
            m_list.Add( new TcpSrvTestSrvside(*this, domain.getMsgThread(), TcpSrvTestSrvsideProfile::Create()) );
            m_list.Add( new TcpSockTestSrvside(*this, domain.getMsgThread(), TcpSockTestSrvsideProfile::Create()) );

            Log("Press any key to exit...");

            m_timer.Start(100, true);
        }

    };

}

// ---------------------------------------------------------

namespace SockTest
{

    void DumpTest()
    {
        QByteArray arr;
        arr.push_back(1);
        arr.push_back(0x10);
        arr.push_back(0x1f);

        std::cout << PcUtils::DumpQByteArray(arr) << std::endl;
    }

    void RunPcBody()
    {
        DataStreamTest::Run();
        
        Domain::DomainTestRunner runner(0);

        RunParams params;

        bool res = runner.Run<Test>(params);
        TUT_ASSERT(res);
    }

}
