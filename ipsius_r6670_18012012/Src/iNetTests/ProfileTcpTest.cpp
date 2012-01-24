#include "stdafx.h"

#include "ProfileTcpTest.h"

namespace 
{
    typedef enum {
        CMaxDuration = 0,
        FinishedBase = 50,// используется для логирования
        CCountClientSocket = 50,
        PayloadSize = 40,
        CSendInterval = 10,        
        CFinishInterval = 100,        
        CRepeatCount = 5        
    } ConstTcpTest;
};

namespace iNetTests
{
    ProfileTcpTest::ProfileTcpTest(iCoreTests::ILoggable &log, const Utils::HostInf &host) :
        m_maxDuration(CMaxDuration),
        m_countClientSocket(CCountClientSocket),
        m_payloadSize(PayloadSize),
        m_sendInterval(CSendInterval),
        m_finishInterval(CFinishInterval),
        m_repeatCount(CRepeatCount),
        m_log(log), 
        m_address(host)
    {}

    //----------------------------------------------------------            

    iCoreTests::ILoggable &ProfileTcpTest::getLog()
    {
        return m_log;
    }

    //----------------------------------------------------------            

    void ProfileTcpTest::SaveTestResult(const iNetTests::DataFlowInf &data)
    {
        m_data.Add(data);
    }

    //----------------------------------------------------------            

    bool ProfileTcpTest::TestIsOk() const
    {
        return m_data.IsOk();
    }
    
    //----------------------------------------------------------            

    void ProfileTcpTest::LogFinishedSocket(int finishedSocket)
    {
        if(finishedSocket % FinishedBase)
            return;
        std::stringstream stream;
        stream << "Finished sockets: " << finishedSocket << std::endl;
        getLog().Log(stream.str());
    }


    //----------------------------------------------------------            
    
    std::string ProfileTcpTest::Result()       
    {
        return m_data.Str();
    }
    
    //----------------------------------------------------------            

    int ProfileTcpTest::getCountClient() const
    {
        return m_countClientSocket;
    }

    //----------------------------------------------------------            

    const Utils::HostInf &ProfileTcpTest::PeerHostInf() const
    {
        return m_address;
    }

    //----------------------------------------------------------            

    int ProfileTcpTest::getMaxTestDuration() const 
    {
        return m_maxDuration; 
        /*
        return getClientStartInterval(getCountClient()) +  
        getServerStartInterval(getCountClient()) + 
        getSendInterval(getCountClient())*getRepeatCount() + 
        getFinishInterval(getCountClient());
        */
    }

    //----------------------------------------------------------            

    int ProfileTcpTest::getPayloadSize() const 
    {
        return m_payloadSize;
    }

    //----------------------------------------------------------            

    int ProfileTcpTest::getSendInterval() const 
    {
        return m_sendInterval;
    }

    //----------------------------------------------------------            

    int ProfileTcpTest::getFinishInterval() const 
    {
        return m_finishInterval;
    }

    //----------------------------------------------------------                        

    int ProfileTcpTest::getRepeatCount() const 
    {
        return m_repeatCount;
    }

};

