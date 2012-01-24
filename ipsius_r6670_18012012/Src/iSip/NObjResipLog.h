#pragma once

#include "stdafx.h"
#include "undeferrno.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "Utils/AtomicTypes.h"
#include "rutil/Logger.hxx"
#include "iLog/LogWrapper.h"


namespace iSip
{
	class NObjResipLog : public Domain::NamedObject, 
		resip::ExternalLogger
	{
		Q_OBJECT;

		typedef NObjResipLog T;

	public:
		
		NObjResipLog (Domain::IDomain *pDomain, const Domain::ObjectName &name); 
		~NObjResipLog();

		static void CreateResipLog(Domain::IDomain &domain);

		enum TraceType
		{
			None = -1,
			Critical,
			Error,
			Warning,
			Info,
			Debug,
			Stack,
			StdErr,
			Bogus
		};

		Q_ENUMS(TraceType);

		enum Subsystem
		{
			All,
			Dns,
			Dum,
			Presence,
			Sdp,
			Sip,
			Transaction,
			Transport,
			Stats
		};

		Q_ENUMS(Subsystem);

		Q_PROPERTY(TraceType Dns READ m_dnsTraceType);

		Q_PROPERTY(TraceType Dum READ m_dumTraceType);
		
		Q_PROPERTY(TraceType Presence READ m_presenceTraceType);

		Q_PROPERTY(TraceType Sdp READ m_sdpTraceType);
		
		Q_PROPERTY(TraceType Sip READ m_sipTraceType);

		Q_PROPERTY(TraceType Transaction READ m_transactionTraceType);

		Q_PROPERTY(TraceType Transport READ m_transportTraceType);
		
		Q_PROPERTY(TraceType Stats READ m_statsTraceType);

		Q_INVOKABLE void SetTraceType(Subsystem susystem, TraceType type);


	// resip::ExternalLogger
	private:
		bool operator()(resip::Log::Level level,
			const resip::Subsystem& subsystem, 
			const resip::Data& appName,
			const char* file,
			int line,
			const resip::Data& message,
			const resip::Data& messageWithHeaders);

	private:

		void onResipLog(const std::string &);

		void SetAllLocalProperty(TraceType type);

	private:
		
		TraceType m_dnsTraceType;
		TraceType m_dumTraceType;
		TraceType m_presenceTraceType;
		TraceType m_sdpTraceType;
		TraceType m_sipTraceType;
		TraceType m_transactionTraceType;
		TraceType m_transportTraceType;
		TraceType m_statsTraceType;

		iLogW::LogRecordTag m_infoTag;
    };
};

