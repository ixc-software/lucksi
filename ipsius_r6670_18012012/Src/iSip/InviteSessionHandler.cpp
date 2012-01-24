#include "stdafx.h"
#include "iSip/undeferrno.h"
#include "InviteSessionHandler.h"
#include "resip/dum/ServerInviteSession.hxx"
#include "resip/dum/ClientInviteSession.hxx"
#include "resip/dum/AppDialog.hxx"
#include "Utils/ErrorsSubsystem.h"


namespace
{
	iSip::IInviteSessionEvent* SafeCast(resip::AppDialog *ptr)
	{
		iSip::IInviteSessionEvent *handler = dynamic_cast<iSip::IInviteSessionEvent*>(ptr);

		ESS_ASSERT(handler != 0);

		return handler;
	}

}

namespace iSip
{
	using resip::ClientInviteSessionHandle;
	using resip::ServerInviteSessionHandle;
	using resip::InviteSessionHandle;
	using resip::InviteSession;
	using resip::SipMessage;
	using resip::SdpContents;

	// -------------------------------------------------------------------------------------
    // implement of InviteSessionHandler

	void InviteSessionHandler::onNewSession(
		ClientInviteSessionHandle h, InviteSession::OfferAnswerType oat, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onNewSession(h, oat, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onNewSession(
		ServerInviteSessionHandle h, InviteSession::OfferAnswerType oat, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onNewSession(h, oat, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onFailure(ClientInviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onFailure(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onEarlyMedia(
		ClientInviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
	{
		SafeCast(h->getAppDialog().get())->onEarlyMedia(h, msg, sdp);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onProvisional(ClientInviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onProvisional(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onConnected(ClientInviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onConnected(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onConnected(InviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onConnected(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onConnectedConfirmed(InviteSessionHandle h, const SipMessage &msg)
	{
		SafeCast(h->getAppDialog().get())->onConnectedConfirmed(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onStaleCallTimeout(ClientInviteSessionHandle h)
	{
		SafeCast(h->getAppDialog().get())->onStaleCallTimeout(h);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onTerminated(InviteSessionHandle h, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg)
	{
		SafeCast(h->getAppDialog().get())->onTerminated(h, reason, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onRedirected(ClientInviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onRedirected(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onAnswer(
		InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
	{
		SafeCast(h->getAppDialog().get())->onAnswer(h, msg, sdp);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onOffer(
		InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
	{
		SafeCast(h->getAppDialog().get())->onOffer(h, msg, sdp);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onOfferRequired(InviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onOfferRequired(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onOfferRejected(
		InviteSessionHandle h, const SipMessage* msg)
	{
		SafeCast(h->getAppDialog().get())->onOfferRejected(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onOfferRequestRejected(
		InviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onOfferRequestRejected(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onRemoteSdpChanged(
		InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
	{
		SafeCast(h->getAppDialog().get())->onRemoteSdpChanged(h, msg, sdp);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onInfo(InviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onInfo(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onInfoSuccess(InviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onInfoSuccess(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onInfoFailure(InviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onInfoFailure(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onRefer(
		InviteSessionHandle h, resip::ServerSubscriptionHandle ssh, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onRefer(h, ssh, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onReferAccepted(
		InviteSessionHandle h, resip::ClientSubscriptionHandle csh, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onReferAccepted(h, csh, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onReferRejected(InviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onReferRejected(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onReferNoSub(InviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onReferNoSub(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onMessage(InviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onMessage(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onMessageSuccess(InviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onMessageSuccess(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onMessageFailure(InviteSessionHandle h, const SipMessage& msg)
	{
		SafeCast(h->getAppDialog().get())->onMessageFailure(h, msg);
	}

	// -------------------------------------------------------------------------------------

	void InviteSessionHandler::onForkDestroyed(ClientInviteSessionHandle h)
	{
		SafeCast(h->getAppDialog().get())->onForkDestroyed(h);
	}
};
