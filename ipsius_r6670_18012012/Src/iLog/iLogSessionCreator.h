#ifndef _I_LOG_SESSION_CREATOR_
#define _I_LOG_SESSION_CREATOR_

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "iLog/LogCommonImplHelpers.h"

namespace iLogW
{
	class LogSession;
	using iLogCommon::LogString;

	class ILogSessionCreator : public Utils::IBasicInterface
	{	
	public:
		virtual LogSession *CreateSessionExt(const LogString &name, 
			bool isActive = false) = 0;

		LogSession *CreateSession(const std::string &name, 
			bool isActive = false)
		{
			return CreateSessionExt(iLogCommon::LogStringConvert::To(name), isActive);
		}

	};
};

#endif
