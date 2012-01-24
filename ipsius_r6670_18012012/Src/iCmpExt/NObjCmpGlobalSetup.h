#ifndef _NOBJ_CMP_GLOBAL_SETUP_H_
#define _NOBJ_CMP_GLOBAL_SETUP_H_

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"
#include "Utils/HostInf.h"

namespace iCmp	{ struct BfGlobalSetup; };

namespace iCmpExt
{
	class NObjCmpGlobalSetup : public Domain::NamedObject,
		public DRI::INonCreatable
	{
		Q_OBJECT;
		typedef NObjCmpGlobalSetup T;

	public:
		NObjCmpGlobalSetup(Domain::NamedObject *pParent, const Domain::ObjectName &name);
		~NObjCmpGlobalSetup();

		Q_PROPERTY(bool DevTraceInd READ m_devTraceInd WRITE DevTraceInd);
		Q_PROPERTY(QString DevTraceHost READ DevTraceHost WRITE DevTraceHost);
		Q_PROPERTY(bool DevWatchdogEnabled READ m_watchdogEnabled WRITE WatchdogEnabled);
		Q_PROPERTY(bool DevResetOnFatalError READ m_resetOnFatalError WRITE RestartOnError);
		Q_PROPERTY(bool SetupNullEcho READ m_setupNullEcho WRITE m_setupNullEcho);
		Q_PROPERTY(bool AsyncDeleteBody READ m_asyncDeleteBody WRITE m_asyncDeleteBody);

		Q_PROPERTY(bool AsyncDropConnOnException READ m_asyncDropConnOnException WRITE m_asyncDropConnOnException);
		Q_PROPERTY(bool DoSafeRefTest READ m_doSafeRefTest WRITE m_doSafeRefTest);

		iCmp::BfGlobalSetup GlobalSetupInf() const;

	private:
		void DevTraceInd(bool par); 
		QString DevTraceHost() const;
		void DevTraceHost(QString par); 
		void WatchdogEnabled(bool par);
		void RestartOnError(bool par);

	private:
		bool m_devTraceInd;
		Utils::HostInf m_devLogHost;
		bool m_watchdogEnabled;
		bool m_resetOnFatalError;
		bool m_setupNullEcho;
		bool m_asyncDeleteBody;
		bool m_asyncDropConnOnException;
		bool m_doSafeRefTest;

	};
};

#endif


