#ifndef _LOG_MANAGER_OUTPUT_I_H_
#define _LOG_MANAGER_OUTPUT_I_H_

#include "stdafx.h"
#include "Utils/HostInf.h"
#include "LogOutSettings.h"


namespace iLogW
{
    class LogOutput;
    class ILogRecordFormater;
    class LogBasicStream;

    class LogOutputManager : boost::noncopyable
    {
    public:
        LogOutputManager(LogOutput &,
            boost::shared_ptr<ILogRecordFormater>, 
            const LogOutSettings &);

        void ChangeFormater(boost::shared_ptr<ILogRecordFormater> formater)
        {
            m_formater = formater;
        }
		const LogOutSettings &Settings() const 
		{
			return m_settings;
		}
		void TraceToCout(bool traceInd);
        void TraceToCoutOn(boost::shared_ptr<ILogRecordFormater>);
        
		void NameLogFile(const std::string &fileName);
        void DirectSaveToFile(bool directSaveToFile = true);
        void TraceToFile(bool traceInd);
        void TraceToFileOn(boost::shared_ptr<ILogRecordFormater>);
        
		void DstUdpTrace(const Utils::HostInf &host);
		void TraceToUdp(bool traceInd);
		void TraceToUdpOn(boost::shared_ptr<ILogRecordFormater> formater);

        void UartPort(int portNumber);
        void TraceToUart(bool traceInd);
        void TraceToUartOn(boost::shared_ptr<ILogRecordFormater> formater);

    private:
        void ClearStream(boost::scoped_ptr<LogBasicStream> &stream);
    private:
		LogOutSettings m_settings;
        LogOutput &m_logOutput;
        boost::shared_ptr<ILogRecordFormater> m_formater;
        boost::scoped_ptr<LogBasicStream> m_coutStream;
        boost::scoped_ptr<LogBasicStream> m_fileStream;
        boost::scoped_ptr<LogBasicStream> m_udpStream;
        boost::scoped_ptr<LogBasicStream> m_uartStream;
    };
};

#endif
