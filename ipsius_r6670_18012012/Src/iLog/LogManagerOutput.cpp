#include "stdafx.h"
#include "LogWrapperLibrary.h"
#include "iLog/LogUdpStream_i.h" 
#include "LogManagerOutput.h"


namespace iLogW
{
    LogOutputManager::LogOutputManager(LogOutput &logOutput,
        boost::shared_ptr<ILogRecordFormater> formater, 
        const LogOutSettings &settings) :
        m_logOutput(logOutput),
        m_formater(formater)
    {
        if (settings.Cout().TraceInd) TraceToCout(true);
        
        if (settings.File().TraceInd)
        {
            NameLogFile(settings.File().NameLogFile);
			DirectSaveToFile(settings.File().DirectSaveToFile);
			TraceToFile(true);
        }
			
        if(settings.Udp().TraceInd)	
		{
            DstUdpTrace(settings.Udp().DstHost);
			TraceToUdp(true);
		}

        if(settings.Uart().TraceInd)
		{
			UartPort(settings.Uart().Port);
			TraceToUart(true);
		}
    }

    // -------------------------------------------------------------------------

    void LogOutputManager::TraceToCout(bool traceInd)
    {
        if (traceInd)
			TraceToCoutOn(m_formater);
		else
		{
			m_settings.Cout().TraceInd = false;
			ClearStream(m_coutStream);
		}
    }

    // -------------------------------------------------------------------------

    void LogOutputManager::TraceToCoutOn(boost::shared_ptr<ILogRecordFormater> formater)
    {
        ClearStream(m_coutStream);
		m_settings.Cout().TraceInd = true;
        m_coutStream.reset(new LogCoutStream(formater));
        m_logOutput.Register(m_coutStream.get());
    }

    // -------------------------------------------------------------------------

	void LogOutputManager::NameLogFile(const std::string &fileName)
	{
		ESS_ASSERT(!m_settings.File().TraceInd);

		m_settings.File().NameLogFile = fileName;
	}

	// -------------------------------------------------------------------------

	void LogOutputManager::DirectSaveToFile(bool directSaveToFile)
	{
		ESS_ASSERT(!m_settings.File().TraceInd);

		m_settings.File().DirectSaveToFile = directSaveToFile;
	}

	// -------------------------------------------------------------------------
        
	void LogOutputManager::TraceToFile(bool traceInd)
    {
        if (traceInd)
			TraceToFileOn(m_formater);
		else
		{
			m_settings.File().TraceInd = false;
			ClearStream(m_fileStream);
		}
    }

	// -------------------------------------------------------------------------

	void LogOutputManager::DstUdpTrace(const Utils::HostInf &host)
	{
		ESS_ASSERT(!m_settings.Udp().TraceInd);

		m_settings.Udp().DstHost = host;
	}

	// -------------------------------------------------------------------------

	void LogOutputManager::TraceToUdp(bool traceInd)
	{
		if (traceInd)
			TraceToUdpOn(m_formater);
		else
		{
			m_settings.Udp().TraceInd = false;
			ClearStream(m_udpStream);
		}
	}
	
	// -------------------------------------------------------------------------

	void LogOutputManager::TraceToUdpOn(boost::shared_ptr<ILogRecordFormater> formater)
	{
		ESS_ASSERT(!m_settings.Udp().DstHost.Empty() && "Address of destination must be present");

        m_settings.Udp().TraceInd = true;
    	ClearStream(m_udpStream);
		m_udpStream.reset(new LogUdpStream(m_settings.Udp().DstHost, formater));
		m_logOutput.Register(m_udpStream.get());
	}

    // -------------------------------------------------------------------------

	void LogOutputManager::UartPort(int portNumber)
	{
		ESS_ASSERT(!m_settings.Uart().TraceInd);
		
		m_settings.Uart().Port = portNumber;
	}

	// -------------------------------------------------------------------------

    void LogOutputManager::TraceToUart(bool traceInd)
    {
		if (traceInd)
			TraceToUartOn(m_formater);
		else
		{
			m_settings.Uart().TraceInd = false;
			ClearStream(m_uartStream);
		}
	}

    // -----------------------------------------------------------------------


    void LogOutputManager::ClearStream(boost::scoped_ptr<LogBasicStream> &stream)
    {
        if(stream == 0) return;

        m_logOutput.Unregister(stream.get());
        stream.reset();
    }


};

