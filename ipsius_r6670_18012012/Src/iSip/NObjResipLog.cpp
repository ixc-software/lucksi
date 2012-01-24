#include "stdafx.h"
#include "NObjResipLog.h"

namespace {
	
	resip::Log::Level ResipLevel(iSip::NObjResipLog::TraceType type)
	{
		switch(type)
		{
			case iSip::NObjResipLog::None: return resip::Log::None;
			case iSip::NObjResipLog::Critical: return resip::Log::Crit;
			case iSip::NObjResipLog::Error: return resip::Log::Err;
			case iSip::NObjResipLog::Warning : return resip::Log::Warning;
			case iSip::NObjResipLog::Info: return resip::Log::Info;
			case iSip::NObjResipLog::Debug: return resip::Log::Debug;
			case iSip::NObjResipLog::Stack: return resip::Log::Stack;
			case iSip::NObjResipLog::StdErr: return resip::Log::StdErr;
			case iSip::NObjResipLog::Bogus: return resip::Log::Bogus;
		};
		return resip::Log::None;
	}

	resip::Subsystem ResipSubsystem(iSip::NObjResipLog::Subsystem susystem)
	{
		using iSip::NObjResipLog;

		if (susystem == NObjResipLog::Dns)			return resip::Subsystem::DNS;
		if (susystem == NObjResipLog::Dum)			return resip::Subsystem::DUM;
		if (susystem == NObjResipLog::Presence)		return resip::Subsystem::PRESENCE;
		if (susystem == NObjResipLog::Sdp) return	resip::Subsystem::SDP;
		if (susystem == NObjResipLog::Sip) return	resip::Subsystem::SIP;
		if (susystem == NObjResipLog::Transaction)	return resip::Subsystem::TRANSACTION;
		if (susystem == NObjResipLog::Transport)	return resip::Subsystem::TRANSPORT;
		if (susystem == NObjResipLog::Stats)		return resip::Subsystem::STATS;

		return resip::Subsystem::NONE;
	}

};

namespace iSip
{

	NObjResipLog::NObjResipLog(Domain::IDomain *pDomain, const Domain::ObjectName &name) : 
		Domain::NamedObject(pDomain, name)
	{
		if(name.Compare(Domain::ObjectName("ResipLog")) != 0)
		{
			ThrowRuntimeException("Name this object should be 'Resip log'");
		}

		SetAllLocalProperty(None);
		resip::Log::initialize(resip::Log::OnlyExternal, resip::Log::None, "ipsius", *this);
		m_infoTag = Log().RegisterRecordKind(L"Resip");
	}

	// -------------------------------------------------------------------------------

	NObjResipLog::~NObjResipLog()
	{
		SetTraceType(All, None);
	}

	// -------------------------------------------------------------------------------

	void NObjResipLog::CreateResipLog(Domain::IDomain &domain)
	{
		Domain::ObjectName resipLogName("ResipLog");
		iSip::NObjResipLog *resipLog = 
			dynamic_cast<iSip::NObjResipLog*>(domain.getDomain().FindFromRoot(resipLogName));

		if (resipLog == 0) resipLog = new iSip::NObjResipLog (&domain, resipLogName);
	}

	// -------------------------------------------------------------------------------

	void NObjResipLog::SetTraceType(Subsystem susystem, TraceType type)
	{
		if (susystem == All) 
			resip::Log::setLevel(ResipLevel(type));
		else
                        ;//resip::Log::setLevel(ResipLevel(type), ResipSubsystem(susystem));
	}

	// -------------------------------------------------------------------------------
	// resip::ExternalLogger

	bool NObjResipLog::operator()(resip::Log::Level level,
		const resip::Subsystem& subsystem, 
		const resip::Data& appName,
		const char* file,
		int line,
		const resip::Data& message,
		const resip::Data& messageWithHeaders)
	{
		if(!Log().LogActive())	return false;

		std::ostringstream out;
		
		out << "Level: " << level 
			<< "; Subsystem: " << subsystem.getSubsystem().c_str()
			<< "; File: " << file
			<< "; Line: " << line 
			<< "\r\n" << messageWithHeaders.c_str();


		PutMsg(this, &T::onResipLog, out.str());

		return false;
	}

	// -------------------------------------------------------------------------------

	void NObjResipLog::onResipLog(const std::string &str)
	{
		if(!Log().LogActive())	return;

		Log() << m_infoTag << str << iLogW::EndRecord;
	}

	// -------------------------------------------------------------------------------

	void NObjResipLog::SetAllLocalProperty(TraceType type)
	{
		m_dnsTraceType = type;
		m_dumTraceType = type;
		m_sdpTraceType = type;
		m_sipTraceType = type;
		m_presenceTraceType		= type;
		m_transactionTraceType	= type;
		m_transportTraceType	= type;
		m_statsTraceType		= type;
	}
};

