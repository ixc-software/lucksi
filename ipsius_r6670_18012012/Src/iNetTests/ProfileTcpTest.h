#ifndef _PROFILE_TCP_TEST_H_
#define _PROFILE_TCP_TEST_H_

#include "stdafx.h"

#include "iCoreTests/iCoreLogTest.h"
#include "Utils/HostInf.h"
#include "iNetTests/DataFlowInf.h"

namespace iNetTests
{

    class ProfileTcpTest
    {
        int m_maxDuration;
        int m_countClientSocket;
        int m_payloadSize;
        int m_sendInterval;        
        int m_finishInterval;        
        int m_repeatCount;        

        iCoreTests::ILoggable &m_log;
        Utils::HostInf m_address;
        iNetTests::DataFlowInf m_data;
    public:

        ProfileTcpTest(iCoreTests::ILoggable &log, const Utils::HostInf &host);

        iCoreTests::ILoggable &getLog();

        void SaveTestResult(const iNetTests::DataFlowInf &data);

        bool TestIsOk() const;
        void LogFinishedSocket(int finishedSocket);       
        std::string Result();        
        int getCountClient() const;
        const Utils::HostInf &PeerHostInf() const;
        int getMaxTestDuration() const;
        int getPayloadSize() const; 
        int getSendInterval() const; 
        int getFinishInterval() const; 
        int getRepeatCount() const; 
    };
};

#endif

