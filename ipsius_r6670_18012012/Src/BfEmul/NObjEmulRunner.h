#ifndef _NOBJ_EMUL_RUNNER_H_
#define _NOBJ_EMUL_RUNNER_H_

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "Domain/NObjLogSettings.h"
#include "Utils/threadtaskrunner.h"
#include "Utils/ManagedList.h"
#include "E1App/RtpPortRangeList.h"
#include "E1App/NetworkSettings.h"

#include "NObjBoardEmul.h"

namespace BfEmul
{
	class NObjBoardEmulPair;
	using Platform::dword;

	class NObjEmulRunner : public Domain::NamedObject
	{		 
		Q_OBJECT;
		typedef NObjEmulRunner T;
	public:
		NObjEmulRunner(Domain::IDomain *pDomain, const Domain::ObjectName &name);			 
		~NObjEmulRunner();
		
		Q_INVOKABLE void InitRtpRange(int min, int max);
		Q_PROPERTY(int MinRtpPort READ MinRtpPort);
		int MinRtpPort() const
		{
			return (m_list == 0) ? 0 : m_list->MinRtpPort();
		}

		Q_PROPERTY(int MaxRtpPort READ MaxRtpPort);
		int MaxRtpPort() const
		{
			return (m_list == 0) ? 0 : m_list->MaxRtpPort();
		}
		Q_INVOKABLE void CreateEmulPair(const QString &name, int boardNumber1, int boardNumber2);
        Q_INVOKABLE void CreateEmul(const QString &name, int boardNumber);

		Q_INVOKABLE void StartAllEmul(DRI::IAsyncCmd *pAsyncCmd);
		Q_INVOKABLE void StopAllEmul(DRI::IAsyncCmd *pAsyncCmd);

        //Q_INVOKABLE void DeleteAllEmul(); 
        //Q_INVOKABLE void DeleteAllEmulPair(); 


	public:
		iLogW::LogSettings LogSettings();
		
        E1App::IRtpRangeCreator &RtpRangeCreator()
		{
			return *m_list;
		}
        E1App::NetworkSettings GetNetworkSettings() const;
	// Domain::NamedObject
	private:        
		void OnObjectDelete(DRI::IAsyncCmd *pAsyncCmd);
	private:        
		void EnableAll(bool enabled);
		void OnTimer(iCore::MsgTimer *pT);
        bool EmulExist(int boardNum);
        Utils::ThreadTaskRunner &TaskRunner()
        {
            return m_runner;
        }

	private:
		Domain::NObjLogSettings* m_logSettings;
        boost::scoped_ptr<E1App::RtpPortRangeList> m_list;
		Utils::ThreadTaskRunner m_runner;
		int m_timerCycle;
		iCore::MsgTimer m_timer;

        std::vector<NObjBoardEmulPair*> m_emulPair;
        std::vector<NObjBoardEmul*> m_emul;
	};
};

#endif
