#ifndef LOG_INTF_H
#define LOG_INTF_H

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "Utils/QtHelpers.h"
#include "Utils/ErrorsSubsystem.h"
#include "ISDN/IsdnUtils.h"

#include "iLog/LogWrapper.h"

namespace iLogW {   class ILogSessionToLogStore; }

namespace ISDN
{
    using IsdnUtils::ToLogString;

	class IIsdnLog : public Utils::IBasicInterface // implementation in legatee of IsdnInfra
	{
	public:
		
        virtual iLogW::ILogSessionToLogStore& getLogStoreIntf() = 0;
        virtual iLogW::LogSessionProfile& getLogSessionProfile() = 0;

	};

    //-------------------------------------------

    // log switcher for inheritance in loggable class
    class ILoggable /*: boost::noncopyable*/
	{
	public:

        ILoggable(IIsdnLog& log, std::string name = "", bool traceOn = false)
            : m_name(name),
            m_session(log.getLogStoreIntf(), ToLogString(name), log.getLogSessionProfile()),
            m_log(log)
        {
            m_session.LogActive(traceOn);
        }

        // принимает параметры исходной сессии дописывая имя
        explicit ILoggable(const ILoggable& log, const std::string& postfix)
            : m_name(log.m_name + postfix),
            m_session(log.m_log.getLogStoreIntf(), ToLogString(m_name), log.m_log.getLogSessionProfile()),
            m_log(log.m_log)            
        {
            ESS_ASSERT(&log != this);
            m_session.LogActive(log.getTraceOn());
        }

        bool getTraceOn()const 
        {
            return m_session.LogActive();
        }

        void setTraceOn(bool traceOn) 
        {
            m_session.LogActive(traceOn);
        }

		void DoLog(const QString &in, iLogW::LogRecordTag recordKind = iLogW::LogRecordTag())
		{
            m_session.Add( ToLogString(in), recordKind );            
		}

        iLogW::LogRecordTag RegNewTag(const std::string& tagName)
        {
            return m_session.RegisterRecordKind( ToLogString(tagName) );
        }      

        const std::string& getName() const
        {
            return m_name;
        }

	private:       

        std::string m_name;
        iLogW::LogSession m_session;        		
        IIsdnLog& m_log;
		//bool m_traceOn;
        //std::string m_prefix; 
	};

    //-------------------------------------------

      

	class LogWriter//
	{
        iLogW::LogRecordTag m_recordKind;

	public:

		LogWriter(ILoggable* pLog, iLogW::LogRecordTag recordKind = iLogW::LogRecordTag())
            : m_recordKind(recordKind),
            workedOut(false),
            m_pLog(pLog)
        {}

		~LogWriter()
        {
            if (!workedOut)
            m_pLog->DoLog( Utils::StringToQString(m_sstring.str()), m_recordKind );
        }

        void SendContent()
        {
            ESS_ASSERT(!workedOut);
            m_pLog->DoLog( Utils::StringToQString(m_sstring.str()), m_recordKind );
            workedOut = true;
        }
		
        std::stringstream & Write()	
        {
            ESS_ASSERT(!workedOut);
            return m_sstring;
        }

	private:
        
        bool workedOut;
		ILoggable* m_pLog;
		std::stringstream m_sstring;

	};


}//ISDN

#endif
