#ifndef _CMP_PPOINT_LOG_H_
#define _CMP_PPOINT_LOG_H_

#include "iLog/LogWrapper.h"

namespace iCmpExt
{

	class CmpPointLog : boost::noncopyable
	{
	public:

		CmpPointLog(iLogW::ILogSessionCreator &logCreator,
			const std::string &name)
		{
			m_log.reset(logCreator.CreateSession(name, true));
			m_infoTag = m_log->RegisterRecordKind(L"Info");
		}

		bool LogActive() const 
		{
			return m_log != 0 && m_log->LogActive(m_infoTag);
		}
			
		void Log(const std::string &msg)
		{
			*m_log << m_infoTag << msg << iLogW::EndRecord;
		}

		iLogW::LogSession &Log() 
		{
			return *m_log << m_infoTag;
		}
    private:
		// boost::shared_ptr<iLogW::LogSession> m_log;
		boost::scoped_ptr<iLogW::LogSession> m_log;
		iLogW::LogRecordTag m_infoTag;
	};
};

#endif






