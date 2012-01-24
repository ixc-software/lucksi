#include "stdafx.h"
#include "NObjCmpGlobalSetup.h"
#include "iCmp/ChMngProto.h"
#include "Utils/GetDefLocalHost.h"


namespace 
{
	const int CDefDevLogPort = 56001;
};

namespace iCmpExt
{
	NObjCmpGlobalSetup::NObjCmpGlobalSetup(Domain::NamedObject *pParent, const Domain::ObjectName &name) :
		NamedObject(&pParent->getDomain(), name, pParent)
	{
		PropertyWriteEvent(pParent);
		m_devTraceInd = false;
		m_watchdogEnabled = true;
		m_resetOnFatalError = true;
		m_setupNullEcho = false;
		m_asyncDeleteBody = false;
		m_asyncDropConnOnException = false;
		m_doSafeRefTest = false;

		m_devLogHost = Utils::GetDefLocalHostInf(CDefDevLogPort);
	}

	// -------------------------------------------------------------------------------

	NObjCmpGlobalSetup::~NObjCmpGlobalSetup()
	{}

	// -------------------------------------------------------------------------------

	void NObjCmpGlobalSetup::DevTraceInd(bool par)
	{
		m_devTraceInd = par;
	}

	// -------------------------------------------------------------------------------

	QString NObjCmpGlobalSetup::DevTraceHost() const
	{
		return m_devLogHost.ToString().c_str();
	}

	// -------------------------------------------------------------------------------

	void NObjCmpGlobalSetup::DevTraceHost(QString par)
	{
		m_devLogHost = Utils::HostInf::FromString(par.toStdString());
	}

	// -------------------------------------------------------------------------------

	void NObjCmpGlobalSetup::WatchdogEnabled(bool par)
	{
		m_watchdogEnabled = par;
	}

	// -------------------------------------------------------------------------------

	void NObjCmpGlobalSetup::RestartOnError(bool par)
	{
		m_resetOnFatalError = par;
	}

	// -------------------------------------------------------------------------------

	iCmp::BfGlobalSetup NObjCmpGlobalSetup::GlobalSetupInf() const
	{
		iCmp::BfGlobalSetup data;

		data.LogEnable = m_devTraceInd;
		data.UdpHostPort = m_devLogHost.ToString();

		if(data.LogEnable && data.UdpHostPort.empty()) ThrowRuntimeException("Address of device trace is empty.");

		data.WatchdogTimeoutMs = (m_watchdogEnabled) ? 5000 : 0;
		data.ResetOnFatalError = m_resetOnFatalError;
		data.SetupNullEcho	   = m_setupNullEcho;
		data.AsyncDeleteBody = m_asyncDeleteBody;
		data.AsyncDropConnOnException = m_asyncDropConnOnException;
		data.DoSafeRefTest = m_doSafeRefTest;

		return data;
	}

};



