#ifndef __LOGGER__
#define __LOGGER__

// #include "IsdnTestConfig.h"
#include "stdafx.h"
//#include "LogOut.h"
#include "iLog/LogWrapper.h"
#include "Utils/QtHelpers.h"
#include "ISDN/IsdnUtils.h"

namespace IsdnTest
{
    
    using ISDN::IsdnUtils::ShortName;
    using ISDN::IsdnUtils::TypeToShortName;
    using ISDN::IsdnUtils::ToLogString;

    //typedef ISDN::ILoggable Logger;

	class Logger
	{
	public:

        Logger(std::string name, iLogW::ILogSessionToLogStore &out, bool traceOn, 
            iLogW::LogSessionProfile& prof) : 
            m_name(ShortName(Utils::StringToQString(name))),
            m_out(out),
            m_prof(prof),
            //isxActive(traceOn),
            m_session(out, ToLogString(m_name), prof)
            //m_out(out)
        {
            m_session.LogActive(traceOn);
        }

        Logger(const Logger& logger, QString name)
            : m_name(logger.m_name + ShortName(name) ),
            m_out(logger.m_out),
            m_prof(logger.m_prof),
            //isxActive(logger.m_session.getLogActive()),
            m_session(m_out, ToLogString(m_name), m_prof)
        {
            m_session.LogActive( logger.m_session.LogActive() );
        }

        Logger(const Logger& logger, const std::string& name)
            : m_name(logger.m_name + "/" + ShortName(name.c_str()) ),
            m_out(logger.m_out),
            m_prof(logger.m_prof),
            //isxActive(logger.m_session.getLogActive()),
            m_session(m_out, ToLogString(m_name), m_prof)
        {
            m_session.LogActive( logger.m_session.LogActive() );
        }

        template<class TOwner>
        Logger(const Logger& logger, TOwner*)
            : m_name( logger.m_name + "/" + TypeToShortName<TOwner>() ),
            m_out(logger.m_out),
            m_prof(logger.m_prof),
            //isxActive(logger.m_session.getLogActive()),
            m_session(m_out, ToLogString(m_name), m_prof)
        {
            m_session.LogActive( logger.m_session.LogActive() );
        }

		void Log(const std::string &in, iLogW::LogRecordTag recordKind = iLogW::LogRecordTag())//TODO убрать дефолт
		{
            if ( m_session.LogActive() )
                m_session.Add( ToLogString(in), recordKind );              
		}

        void SwitchTrace(bool on)
        {
            m_session.LogActive(on);
        }

        iLogW::LogRecordTag RegNewTag(const char* tagName)
        {
            return m_session.RegisterRecordKind( ToLogString(std::string(tagName)) );
        }

	private:        
       	               
        QString m_name;
        iLogW::ILogSessionToLogStore &m_out;
        iLogW::LogSessionProfile& m_prof;

        iLogW::LogSession m_session;

        //bool isxActive;        
	};

    //---------------

    
	class Writer
	{
	public:

		Writer(Logger& log, iLogW::LogRecordTag recordKind = iLogW::LogRecordTag())
            : m_log(log),
            m_recordKind(recordKind)
        {}

		~Writer(){ m_log.Log( m_ss.str(), m_recordKind ); }

		std::stringstream& Write(){ return m_ss; }

	private:
		
        std::stringstream m_ss;
		Logger & m_log;
        iLogW::LogRecordTag m_recordKind;
	};

}  //namespace IsdnTest

#endif
