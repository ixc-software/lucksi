#include "stdafx.h"
#include "iSip/undeferrno.h"

#include "ServerSubscriptionHandler.h"
#include "RemoteParticipant.h"

#include "Utils/ErrorsSubsystem.h"

#include "resip/stack/SipMessage.hxx"
#include <resip/dum/DialogUsageManager.hxx>
#include <resip/dum/ClientSubscription.hxx>
#include <resip/dum/ServerSubscription.hxx>


namespace iSip
{

	ServerSubscriptionHandler::ServerSubscriptionHandler(
		iLogW::ILogSessionCreator &logCreator,
		resip::DialogUsageManager &dum,
		NewParticipantCreator newParticipantCreator) :
		m_log(logCreator.CreateSession("ServerSubscription", true)),
		m_infoTag(m_log->RegisterRecordKind(L"Info", true)),
		m_warningTag(m_log->RegisterRecordKind(L"Warning", true)),
		m_dum(dum),
		m_newParticipantCreator(newParticipantCreator)
	{
		ESS_ASSERT(!m_newParticipantCreator.empty());

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Created" << iLogW::EndRecord;
		}

	}

	// -------------------------------------------------------------------------------------

	ServerSubscriptionHandler::~ServerSubscriptionHandler()
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Deleted" << iLogW::EndRecord;
		}

	}

	// -------------------------------------------------------------------------------------

	void ServerSubscriptionHandler::onNewSubscription(
		resip::ServerSubscriptionHandle, const resip::SipMessage &msg)
	{
		InfoLog("onNewSubscription(ServerSubscriptionHandle): ", &msg);
	}

	// -------------------------------------------------------------------------------------

	void ServerSubscriptionHandler::onNewSubscriptionFromRefer(
		resip::ServerSubscriptionHandle ss, const resip::SipMessage &msg)
	{
		InfoLog("onNewSubscriptionFromRefer(ServerSubscriptionHandle): ", &msg);

		// Received an out-of-dialog refer request with implicit subscription
		try
		{
			if (!msg.exists(resip::h_ReferTo))
			{
				WarningLog("Received refer w/out a Refer-To: ", &msg);

				ss->send(ss->reject(400));
				return;
			}

			// Check if TargetDialog header is present
			if (msg.exists(resip::h_TargetDialog))
			{
				std::pair<resip::InviteSessionHandle, int> presult;
				presult = m_dum.findInviteSession(msg.header(resip::h_TargetDialog));
				if (!(presult.first == resip::InviteSessionHandle::NotValid()))
				{
					IInviteSessionEvent* participantToRefer = (IInviteSessionEvent*)presult.first->getAppDialog().get();

					participantToRefer->onRefer(presult.first, ss, msg);
					return;
				}
			}

			m_newParticipantCreator(ss, msg);
		}
		catch(resip::BaseException &e)
		{
			WarningLog("onNewSubscriptionFromRefer exception: " + SipUtils::ToString(e));
		}
		catch(...)
		{
			WarningLog("onNewSubscriptionFromRefer unknown exception");
		}
	}

	// -------------------------------------------------------------------------------------

	void ServerSubscriptionHandler::onRefresh(resip::ServerSubscriptionHandle, const resip::SipMessage &msg)
	{
		InfoLog("onRefresh(ServerSubscriptionHandle): ", &msg);
	}

	// -------------------------------------------------------------------------------------

	void ServerSubscriptionHandler::onTerminated(resip::ServerSubscriptionHandle)
	{
		InfoLog("onTerminated(ServerSubscriptionHandle)");
	}

	// -------------------------------------------------------------------------------------

	void ServerSubscriptionHandler::onReadyToSend(resip::ServerSubscriptionHandle, resip::SipMessage&)
	{
	}

	// -------------------------------------------------------------------------------------

	void ServerSubscriptionHandler::onNotifyRejected(resip::ServerSubscriptionHandle, const resip::SipMessage &msg)
	{
		WarningLog("onNotifyRejected(ServerSubscriptionHandle): ", &msg);
	}

	// -------------------------------------------------------------------------------------

	void ServerSubscriptionHandler::onError(resip::ServerSubscriptionHandle, const resip::SipMessage &msg)
	{
		WarningLog("onError(ServerSubscriptionHandle): ", &msg);
	}

	// -------------------------------------------------------------------------------------

	void ServerSubscriptionHandler::onExpiredByClient(
		resip::ServerSubscriptionHandle, const resip::SipMessage &sub, resip::SipMessage &notify)
	{
		InfoLog("onExpiredByClient(ServerSubscriptionHandle): ", &notify);
	}

	// -------------------------------------------------------------------------------------

	void ServerSubscriptionHandler::onExpired(resip::ServerSubscriptionHandle, resip::SipMessage &msg)
	{
		InfoLog("onExpired(ServerSubscriptionHandle): ", &msg);
	}

	// -------------------------------------------------------------------------------------

	bool ServerSubscriptionHandler::hasDefaultExpires() const
	{
		return true;
	}

	// -------------------------------------------------------------------------------------

	boost::uint32_t ServerSubscriptionHandler::getDefaultExpires() const
	{
		return 60;
	}

	// -------------------------------------------------------------------------------------

	void ServerSubscriptionHandler::LogMsg(iLogW::LogRecordTag  &tag,
		const std::string &event,
		const resip::SipMessage *msg)
	{
		if(m_log->LogActive(tag))
		{
			*m_log << tag << event;

			if (msg != 0) *m_log << SipUtils::ToString(msg);

			*m_log << iLogW::EndRecord;
		}

	}


};
