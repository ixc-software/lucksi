#include "stdafx.h"
#include "iLog/LogWrapperLoggers.h"
#include "iLog/LogWrapperInterfaces.h"
#include "Utils/QtHelpers.h"

namespace iLogW
{
	
    LoggerQTextStream::~LoggerQTextStream()
    {
        m_session.AddRecord( Utils::QStringToWString(m_stream.readAll()), m_kind );
    }
		
}  // namespace iLog

