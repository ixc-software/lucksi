#pragma once

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "resip/dum/InviteSessionHandler.hxx"

namespace iSip
{

    class InviteSessionHandler : boost::noncopyable,
        public resip::InviteSessionHandler 
    {
    // implement of InviteSessionHandler
    private:
		void onNewSession(
			resip::ClientInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);

		void onNewSession(
			resip::ServerInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);

		void onFailure(resip::ClientInviteSessionHandle h, const resip::SipMessage& msg);

		void onEarlyMedia(
			resip::ClientInviteSessionHandle, const resip::SipMessage&, const resip::SdpContents&);

		void onProvisional(
			resip::ClientInviteSessionHandle, const resip::SipMessage& msg);

		void onConnected(
			resip::ClientInviteSessionHandle h, const resip::SipMessage& msg);

		void onConnected(resip::InviteSessionHandle, const resip::SipMessage& msg);

		void onConnectedConfirmed(resip::InviteSessionHandle, const resip::SipMessage &msg);

		void onStaleCallTimeout(resip::ClientInviteSessionHandle);

		void onTerminated(
			resip::InviteSessionHandle h, resip::InviteSessionHandler::TerminatedReason reason, const resip::SipMessage* msg);

		void onRedirected(resip::ClientInviteSessionHandle, const resip::SipMessage& msg);

		void onAnswer(
			resip::InviteSessionHandle, const resip::SipMessage& msg, const resip::SdpContents&);

		void onOffer(
			resip::InviteSessionHandle handle, const resip::SipMessage& msg, const resip::SdpContents& offer);

		void onOfferRequired(resip::InviteSessionHandle, const resip::SipMessage& msg);

		void onOfferRejected(resip::InviteSessionHandle, const resip::SipMessage* msg);

		void onOfferRequestRejected(resip::InviteSessionHandle, const resip::SipMessage& msg);

		void onRemoteSdpChanged(
			resip::InviteSessionHandle, const resip::SipMessage& msg, const resip::SdpContents& sdp);

		void onInfo(resip::InviteSessionHandle, const resip::SipMessage& msg);

		void onInfoSuccess(resip::InviteSessionHandle, const resip::SipMessage& msg);

		void onInfoFailure(resip::InviteSessionHandle, const resip::SipMessage& msg);

		void onRefer(
			resip::InviteSessionHandle, resip::ServerSubscriptionHandle, const resip::SipMessage& msg);

		void onReferAccepted(
			resip::InviteSessionHandle, resip::ClientSubscriptionHandle, const resip::SipMessage& msg);

		void onReferRejected(resip::InviteSessionHandle, const resip::SipMessage& msg);

		void onReferNoSub(resip::InviteSessionHandle, const resip::SipMessage& msg);

		void onMessage(resip::InviteSessionHandle, const resip::SipMessage& msg);

		void onMessageSuccess(resip::InviteSessionHandle, const resip::SipMessage& msg);

		void onMessageFailure(resip::InviteSessionHandle, const resip::SipMessage& msg);

		void onForkDestroyed(resip::ClientInviteSessionHandle);
    };


	// part of resip::InviteSessionHandler, use for hiding implenentation 
	class IInviteSessionEvent : Utils::IBasicInterface,
		public resip::InviteSessionHandler 
	{
		// implement of InviteSessionHandler
	public:
		virtual void onNewSession(
			resip::ClientInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg) = 0;

		virtual void onNewSession(
			resip::ServerInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg) = 0;

		virtual void onFailure(resip::ClientInviteSessionHandle h, const resip::SipMessage& msg) = 0;

		virtual void onEarlyMedia(
			resip::ClientInviteSessionHandle, const resip::SipMessage&, const resip::SdpContents&) = 0;

		virtual void onProvisional(
			resip::ClientInviteSessionHandle, const resip::SipMessage& msg) = 0;

		virtual void onConnected(
			resip::ClientInviteSessionHandle h, const resip::SipMessage& msg) = 0;

		virtual void onConnected(resip::InviteSessionHandle, const resip::SipMessage& msg) = 0;

		virtual void onConnectedConfirmed(resip::InviteSessionHandle, const resip::SipMessage &msg) = 0;

		virtual void onStaleCallTimeout(resip::ClientInviteSessionHandle) = 0;

		virtual void onTerminated(
			resip::InviteSessionHandle h, resip::InviteSessionHandler::TerminatedReason reason, const resip::SipMessage* msg) = 0;

		virtual void onRedirected(resip::ClientInviteSessionHandle, const resip::SipMessage& msg) = 0;

		virtual void onAnswer(
			resip::InviteSessionHandle, const resip::SipMessage& msg, const resip::SdpContents&) = 0;

		virtual void onOffer(
			resip::InviteSessionHandle handle, const resip::SipMessage& msg, const resip::SdpContents& offer) = 0;

		virtual void onOfferRequired(resip::InviteSessionHandle, const resip::SipMessage& msg) = 0;

		virtual void onOfferRejected(resip::InviteSessionHandle, const resip::SipMessage* msg) = 0;

		virtual void onOfferRequestRejected(resip::InviteSessionHandle, const resip::SipMessage& msg) = 0;

		virtual void onRemoteSdpChanged(
			resip::InviteSessionHandle, const resip::SipMessage& msg, const resip::SdpContents& sdp) = 0;

		virtual void onInfo(resip::InviteSessionHandle, const resip::SipMessage& msg) = 0;

		virtual void onInfoSuccess(resip::InviteSessionHandle, const resip::SipMessage& msg) = 0;

		virtual void onInfoFailure(resip::InviteSessionHandle, const resip::SipMessage& msg) = 0;

		virtual void onRefer(
			resip::InviteSessionHandle, resip::ServerSubscriptionHandle, const resip::SipMessage& msg) = 0;

		virtual void onReferAccepted(
			resip::InviteSessionHandle, resip::ClientSubscriptionHandle, const resip::SipMessage& msg) = 0;

		virtual void onReferRejected(resip::InviteSessionHandle, const resip::SipMessage& msg) = 0;

		virtual void onReferNoSub(resip::InviteSessionHandle, const resip::SipMessage& msg) = 0;

		virtual void onMessage(resip::InviteSessionHandle, const resip::SipMessage& msg) = 0;

		virtual void onMessageSuccess(resip::InviteSessionHandle, const resip::SipMessage& msg) = 0;

		virtual void onMessageFailure(resip::InviteSessionHandle, const resip::SipMessage& msg) = 0;

		virtual void onForkDestroyed(resip::ClientInviteSessionHandle) = 0;
	};

};



