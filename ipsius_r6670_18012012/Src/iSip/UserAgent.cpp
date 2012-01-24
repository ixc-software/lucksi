#include "stdafx.h"
#include "iSip/undeferrno.h"
#include "UserAgent.h"
#include "UserAgentRegistrator.h"
#include "UserAgentClientSubscription.h"
#include "UserAgentDialogSetFactory.h"
#include "UserAgentServerAuthManager.h"
#include "ConversationManager.h"
#include "SipUtils.h"

#include "Utils/AtomicTypes.h"
#include "Utils/IntToString.h"

#include <resip/dum/ClientAuthManager.hxx>
#include <resip/dum/ClientSubscription.hxx>
#include <resip/dum/ServerSubscription.hxx>
#include <resip/dum/ClientRegistration.hxx>
#include <resip/dum/KeepAliveManager.hxx>
#include <resip/dum/AppDialogSet.hxx>

#if defined(USE_SSL)
#include <resip/stack/ssl/Security.hxx>
#endif
#include <rutil/WinLeakCheck.hxx>

namespace
{
	int CDumPulseTimeout = 50;	

	int CDumProcessTimeout = 5;

	bool CUseRepeatedPoolTimer = true;

	std::string GetUserAgentName()
	{
		static Utils::AtomicInt GUserAgentCount;
		
		return "UserAgent_" + Utils::IntToString(GUserAgentCount.FetchAndAdd(1));
	}
}

namespace iSip
{
	class UserAgent::ShutdownCmd : public resip::DumCommand
	{
	public:
		ShutdownCmd(iSip::UserAgent* userAgent) :
			m_userAgent(userAgent)
		{}

		void executeCommand()
		{
			m_userAgent->shutdownImpl();
		}

		resip::Message* clone() const { assert(0); return 0; }

		EncodeStream& encode(EncodeStream& strm) const { strm << " UserAgentShutdownCmd: "; return strm; }
		EncodeStream& encodeBrief(EncodeStream& strm) const { return encode(strm); }

	private:
		UserAgent* m_userAgent;
	};
}

namespace iSip
{

	UserAgent::UserAgent(iCore::MsgThread &thread,
		iLogW::ILogSessionCreator &logCreator,
		const SipTrustedNodeList &trustedNodeList,
		resip::SharedPtr<UserAgentMasterProfile> profile,
		boost::shared_ptr<ConversationProfile>    defaultConversationProfile,
		resip::AfterSocketCreationFuncPtr socketFunc) :
		iCore::MsgObject(thread),
		m_timer(this, &T::OnPulse),
		m_log(logCreator.CreateSession(GetUserAgentName(), true)),
		m_infoTag(m_log->RegisterRecordKind(L"Info", true)),
		m_warningTag(m_log->RegisterRecordKind(L"Warning", true)),
		m_profile(profile),
#if defined(USE_SSL)
		m_security(new Security(profile->certPath())),
#else
		m_security(0),
#endif
		m_stack(m_security, profile->getAdditionalDnsServers(), &mSelectInterruptor, false /* stateless */, socketFunc),
		m_stackThread(m_stack, mSelectInterruptor),
		m_dum(m_stack),
		m_dumShutdown(false),
		m_registrator(new UserAgentRegistrator(m_dum, *m_log, defaultConversationProfile)),
		m_subscriber (new UserAgentClientSubscription(m_dum)),
		m_conversationManager(new ConversationManager(thread,
			*m_log,
			defaultConversationProfile,
			trustedNodeList,
			m_dum,
			boost::bind(&UserAgentRegistrator::CallsReceiver, m_registrator.get(), _1),
			boost::bind(&UserAgentRegistrator::GetIncomingConversationProfile, m_registrator.get(), _1)))
	{
		m_running = false;

		m_threadContext.Set();

		// Set Enum Suffixes
		m_stack.setEnumSuffixes(profile->getEnumSuffixes());

		// Enable/Disable Statistics Manager
		m_stack.statisticsManagerEnabled() = profile->statisticsManagerEnabled();

		// Install Handlers
		m_dum.setMasterProfile(m_profile);


		m_dum.setClientAuthManager(std::auto_ptr<resip::ClientAuthManager>(new resip::ClientAuthManager));
		m_dum.setKeepAliveManager(std::auto_ptr<resip::KeepAliveManager>(new resip::KeepAliveManager));
		
		// Set AppDialogSetFactory
		std::auto_ptr<resip::AppDialogSetFactory> dsf(new UserAgentDialogSetFactory(m_dum, *m_conversationManager));
		m_dum.setAppDialogSetFactory(dsf);

		// Set UserAgentServerAuthManager
		resip::SharedPtr<resip::ServerAuthManager> uasAuth( new UserAgentServerAuthManager(
				m_dum,
				boost::bind(&UserAgentRegistrator::GetIncomingConversationProfile, m_registrator.get(), _1)));

		m_dum.setServerAuthManager(uasAuth);

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Created" << iLogW::EndRecord;
		}

	}

	// -------------------------------------------------------------------------------------

	UserAgent::~UserAgent()
	{
	   Shutdown();

	   if(m_log->LogActive(m_infoTag))
	   {
		   *m_log << m_infoTag << "Deleted" << iLogW::EndRecord;
	   }

	}

	// -------------------------------------------------------------------------------------

	bool UserAgent::Startup(std::string &error)
	{
		if (!addTransports(error)) return false;

		ESS_ASSERT (!m_running);

		m_running = true;

		m_stackThread.run();

		m_timer.Start(CDumPulseTimeout, CUseRepeatedPoolTimer);

		return true;
	}

	// -------------------------------------------------------------------------------------

	resip::DialogUsageManager& UserAgent::getDialogUsageManager()
	{
		return m_dum;
	}

	// -------------------------------------------------------------------------------------

	IRemoteParticipantCreator& UserAgent::CallCreator()
	{
		return *m_conversationManager;
	}

	// -------------------------------------------------------------------------------------

	IUserAgentRegistrator& UserAgent::Registrator() {	return *m_registrator; }

	// -------------------------------------------------------------------------------------

	const IUserAgentRegistrator& UserAgent::Registrator() const{	return *m_registrator; }

	// -------------------------------------------------------------------------------------

	const UserAgentMasterProfile::TransportList&
		UserAgent::getTransports() const
	{
		return m_transportList;
	}

	// -------------------------------------------------------------------------------------

	void UserAgent::logDnsCache()
	{
	   m_stack.logDnsCache();
	}

	// -------------------------------------------------------------------------------------

	void UserAgent::clearDnsCache()
	{
	   m_stack.clearDnsCache();
	}

	// -------------------------------------------------------------------------------------

	void UserAgent::post(resip::ApplicationMessage &message, unsigned int ms)
	{
	   if (ms > 0)
	   {
		  m_stack.postMS(message, ms, &m_dum);
	   }
	   else
	   {
		  m_dum.post(&message);
	   }
	}

	// -------------------------------------------------------------------------------------

	IUserAgentSubscription* UserAgent::CreateSubscription(
		iLogW::ILogSessionCreator &logCreator,
		IUserAgentSubscriptionEvent &user,
		resip::SharedPtr<resip::UserProfile> profile,
		const resip::Data& eventType,
		const resip::NameAddr& target,
		unsigned int subscriptionTime,
		const resip::Mime& mimeType)
	{
		// Ensure that the request Mime type is supported in the dum profile
		if (!m_profile->isMimeTypeSupported(resip::NOTIFY, mimeType))
		{
			m_profile->addSupportedMimeType(resip::NOTIFY, mimeType);
		}

		return m_subscriber->CreateSubscription(
			logCreator,
			user,
			profile,
			eventType,
			target,
			subscriptionTime);
	}

	// -------------------------------------------------------------------------------------

	resip::SharedPtr<UserAgentMasterProfile> UserAgent::getUserAgentMasterProfile()
	{
		return m_profile;
	}

	// -------------------------------------------------------------------------------------

	void UserAgent::onDumCanBeDeleted()
	{
		m_threadContext.Assert();

		m_dumShutdown = true;
	}

	// -------------------------------------------------------------------------------------

	void UserAgent::Shutdown()
	{
	
		ShutdownCmd* cmd = new ShutdownCmd(this);
		m_dum.post(cmd);

		Platform::dword startTick = Utils::TimerTicks::Ticks();
		// Wait for Dum to shutdown
		while(!m_dumShutdown)
		{
			int timeout = Utils::TimerTicks::Ticks() - startTick;
			if (timeout > m_profile->ForceShutdownTimeoutMs()) break;
				
			process();
		}

		if (!m_dumShutdown) m_dum.forceShutdown(this);

		m_stackThread.shutdown();
		m_stackThread.join();
	}

	// -------------------------------------------------------------------------------------

    UserAgentMasterProfile::TransportInfo GetActualTransportInfo(const resip::Transport &resipTransport)
    {
        UserAgentMasterProfile::TransportInfo transportInfo;
        transportInfo.mProtocol = resipTransport.getTuple().getType();
        transportInfo.mPort = resipTransport.getTuple().getPort();
        transportInfo.mIPVersion = resipTransport.getTuple().isV4() ? resip::V4 : resip::V6;
        transportInfo.mIPInterface = resip::Tuple::inet_ntop(resipTransport.getTuple());
        resip::Data temp = resipTransport.interfaceName();
        transportInfo.mSipDomainname = resipTransport.tlsDomain();

        return transportInfo;
    }

	bool UserAgent::addTransports(std::string &error)
	{

//		m_transportList = m_profile->getTransports();

		for(UserAgentMasterProfile::TransportList::const_iterator i = m_profile->getTransports().begin();
			i != m_profile->getTransports().end(); i++)
		{
            ESS_ASSERT (i->IsCorrectProtocolType() && "Unknown protocol");

			try
			{
                const resip::Transport *transport = 0;

				if ( i->mProtocol == resip::TLS || i->mProtocol == resip::DTLS )
				{
                    const resip::Transport *transport =
                        m_dum.addTransport(i->mProtocol, i->mPort, i->mIPVersion, i->mIPInterface, i->mSipDomainname, resip::Data::Empty, i->mSslType);
				}

				if ( i->mProtocol == resip::UDP || i->mProtocol == resip::TCP )
				{
					transport =
                        m_dum.addTransport(i->mProtocol, i->mPort, i->mIPVersion, i->mIPInterface);
				}

                ESS_ASSERT (transport != 0);

                m_transportList.push_back(GetActualTransportInfo(*transport));
			}
			catch (resip::BaseException& e)
			{
				error = "Caught: " + SipUtils::ToString(e.getMessage()) +
					". Failed to add " + SipUtils::ToString(resip::Tuple::toData(i->mProtocol)) +
					" transport on " + Utils::IntToString(i->mPort);
 				return false;
			}
		}
		return true;
	}

	// -------------------------------------------------------------------------------------

	void UserAgent::shutdownImpl()
	{
		m_conversationManager->Shutdown();
		m_subscriber->Shutdown();
		m_registrator->Shutdown();

		m_dum.shutdown(this);
	}

	// -------------------------------------------------------------------------------------

	void UserAgent::OnPulse(iCore::MsgTimer *pT)
	{
		Utils::TimerTicks timer;

		if (!CUseRepeatedPoolTimer) m_timer.Start(CDumPulseTimeout);

		process();

		m_dumProcessDuration.Add(timer.Get());
	}

	// -------------------------------------------------------------------------------------

	void UserAgent::process()
	{
		if (m_dumShutdown) return;

		while (m_dum.process(CDumProcessTimeout));
	}
}