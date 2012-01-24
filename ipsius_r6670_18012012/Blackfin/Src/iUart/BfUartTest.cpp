#include "stdafx.h"
#include "Platform/Platform.h"

#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/ThreadRunner.h"

#include "AppConfig.h"
#include "AppCpuUsage.h"

#include "AdiDevice.h"
#include "AdiDeviceCommand.h"
#include "AdiDeviceManager.h"
#include "iLog/LogManager.h"
#include "BfUart.h"
#include "BfUartIO.h"
#include "BfUartManagement.h"


namespace 
{

	const int CSizeBuffer = 10;
	const int CCountBuffers = 20;
	const bool CLogToUdp = true;
	
    using namespace BfDev;

    struct AppParams
    {
        AppCpuUsage &CpuThread;

        AppParams(AppCpuUsage &cpuThread) : CpuThread(cpuThread)
        {            
        }
    };
    
    BfUartProfile UartProfileForTest()
    {
		BfUartProfile profile(0, 
				CCountBuffers, 
				CSizeBuffer,
				iUart::Bidirectional,
				iUart::BaudRate115200Hz,
				iUart::DataBits_8,
				iUart::StopBits_1);
		profile.m_logIsEnable = CLogToUdp;
		
		return profile;
    }
    
    
    class Test : boost::noncopyable,
        public iCore::MsgObject
	{
        typedef Test T;
	public:
        Test(iCore::IThreadRunner &runner, AppParams &param) :
            iCore::MsgObject(runner.getThread()),
			m_profile(UartProfileForTest()),
          	m_log(AppConfig::GetLogSettings()),
            m_deviceManager(1),
            m_uart(m_deviceManager, m_profile)
		{
			m_log.Output().TraceToUdpOn(AppConfig::GetLogHost());
			m_session.reset(m_log.CreateLogSesion("TestUart", m_profile.IsLogEnable())),
            m_infoTag = m_session->RegisterRecordKind("Info");

			if(m_session->LogActive())
			{
	            *m_session << m_infoTag << "Created." << iLogW::EndRecord;
			}
                        
            PutMsg(this, &T::onRead);
		}
		
	private:        
	
        void onRead()
        {
        	int size = m_uart.IO().AvailableForRead();

			if(m_session->LogActive() && size)
			{
	            *m_session << m_infoTag << "AccessDataSize = " << size 
 					<< "';" << iLogW::EndRecord;
			}
        	        	
        	if(!size)
        	{
        		PutMsg(this, &T::onRead);
        		return;
        	}

        	std::vector<byte> buffer(size, 0);
           
            int readedData = m_uart.IO().Read(&buffer.at(0), size);

			if(m_session->LogActive())
			{
	            *m_session << m_infoTag << "; ReadedData = " << readedData
	            	<< " Buffer: '";
	            for(int i = 0; i < readedData; ++i)
	            {
	            	*m_session << buffer[i];
	            }	
				*m_session << "';" << iLogW::EndRecord;
			}                        
            //ESS_ASSERT(readedData == size);

            if(readedData)
            {
            	m_uart.IO().Write(buffer, readedData);
            }
            
            PutMsg(this, &T::onRead);
        }

	private:
		BfUartProfile m_profile;
		iLogW::LogManager m_log;
        boost::scoped_ptr<iLogW::LogSession> m_session;
        iLogW::LogRecordTag m_infoTag;
		
        AdiDeviceManager m_deviceManager;
        BfUart m_uart;
	};
	
};

namespace BfDev
{
	void BfUartTest()
	{
	    AppCpuUsage cpuUsage(true);

	    AppParams params(cpuUsage);
		
	    // run
	    iCore::ThreadRunner test(Platform::Thread::NormalPriority, 0);
	    
	    bool res = test.Run<Test>( params, 0, AppConfig::GetThreadRunnerSleepInterval() );
	}
};


