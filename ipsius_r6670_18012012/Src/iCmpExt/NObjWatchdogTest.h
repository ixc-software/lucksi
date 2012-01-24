#ifndef _NOBJ_CMP_TEST_H_
#define _NOBJ_CMP_TEST_H_

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "iCmp/ChMngProto.h"

namespace SBProtoExt	{	class NObjSbpSettings;	};

namespace iCmpExt
{
	class NObjWatchdogTest  :  public Domain::NamedObject
	{
		Q_OBJECT;
		typedef NObjWatchdogTest T;
	public:
		NObjWatchdogTest(Domain::IDomain *pDomain, const Domain::ObjectName &name);
		~NObjWatchdogTest();

		Q_INVOKABLE void SmallWatchdogTest(DRI::IAsyncCmd *pAsyncCmd, const QString &addr);
		Q_INVOKABLE void HugeWatchdogTest(DRI::IAsyncCmd *pAsyncCmd, const QString &addr);

		Q_INVOKABLE void LoopForeverTest(DRI::IAsyncCmd *pAsyncCmd, const QString &addr);
		Q_INVOKABLE void AssertTest(DRI::IAsyncCmd *pAsyncCmd, const QString &addr);
        Q_INVOKABLE void OutOfMemoryTest(DRI::IAsyncCmd *pAsyncCmd, const QString &addr);
        Q_INVOKABLE void EchoHaltTest(DRI::IAsyncCmd *pAsyncCmd, const QString &addr);
		
		Q_INVOKABLE void StateInfo(DRI::IAsyncCmd *pAsyncCmd, const QString &addr);

        Q_INVOKABLE void RunWatchdogLoop(DRI::IAsyncCmd *pAsyncCmd, const QString &addr, int times);

	private:
		void OnDeleteConnection(boost::shared_ptr<Utils::IBasicInterface> );
		void StartGlobalOperation(DRI::IAsyncCmd *pAsyncCmd, const Utils::HostInf &addr, 
			const iCmp::BfGlobalSetup &msg);
		void CompleteOperation(const std::string &desc);

	private:

        class Connection;
        class GlobalReq;
        class StateInfoReq;
        class WatchdogLoop;

		SBProtoExt::NObjSbpSettings *m_sbpSettings;
		boost::shared_ptr<Utils::IBasicInterface> m_operation;
	};
};

#endif


