#include "stdafx.h"
#include "iSip/undeferrno.h"
#include "NObjSipAutotrainer.h"
#include "NObjSipAccessPoint.h"
#include "NObjSipUserAgent.h"
#include "SipUtils.h"
#include "resip/stack/NameAddr.hxx"

#include "Utils/FullEnumResolver.h"

namespace iSip
{

	NObjSipAutotrainer::NObjSipAutotrainer(Domain::IDomain *pDomain, const Domain::ObjectName &name) :
		Domain::NamedObject(pDomain, name)
	{
	}

	// -------------------------------------------------------------------------------------

	NObjSipAutotrainer::~NObjSipAutotrainer()
	{

	}

	// -------------------------------------------------------------------------------------

	QString CommandToString(NObjSipAutotrainer::Commands type, const QString &par)
	{
		bool error = false;
		
		QString str = Utils::EnumResolve(type, &error).c_str();

		ESS_ASSERT(!error);

		if (!par.isEmpty()) str += ": " + par;

		return str + "; ";
	}


	// -------------------------------------------------------------------------------------

	void NObjSipAutotrainer::AddCommandForOutCall(Commands type, const QString &par)
	{
		if (type == Alerting)	ThrowRuntimeException("Wrong command for outgoing call.");
		if (type == Answer)		ThrowRuntimeException("Wrong command for outgoing call.");

		AddCommandToList(m_commandsOutStr, m_commandsOutCall, type, par);
	}

	// -------------------------------------------------------------------------------------

	void NObjSipAutotrainer::ClearCommandsForOutCall()
	{
		m_commandsOutStr.clear();

		m_commandsOutCall.clear();
	}

	// -------------------------------------------------------------------------------------

	void NObjSipAutotrainer::AddCommandForInCall(Commands type, const QString &par)
	{
		if (type == Dial)	ThrowRuntimeException("Wrong command for incoming call.");

		AddCommandToList(m_commandsInStr, m_commandsInCall, type, par);
	}
	
	// -------------------------------------------------------------------------------------
	
	void NObjSipAutotrainer::ClearCommandsForInCall()
	{
		m_commandsInStr.clear();

		m_commandsInCall.clear();
	}

	// -------------------------------------------------------------------------------------

	void NObjSipAutotrainer::StartOutgoingCall()
	{
		if (m_commandsOutCall.empty()) ThrowRuntimeException("Commands list for outgoing call is empty");

		m_calls.Add(new AutotrainerSipCall(getMsgThread(),
			Log(),
			boost::bind(&T::DeleteCall, this, _1),
			m_commandsOutCall));
	}

	// -------------------------------------------------------------------------------------
	// ISipCallManagerEvents

	void NObjSipAutotrainer::OnIncomingParticipant(const ISipAccessPoint *src,
		Utils::SafeRef<IRemoteParticipant> newParticipant,
		const resip::SipMessage &msg,
		bool autoAnswer,
		const ConversationProfile &conversationProfile)
	{
		if (IsLogActive(InfoTag))
		{
			Log(InfoTag) << "Receive incoming call." << iLogW::EndRecord;
		}

		if (m_commandsInCall.empty())
		{
			Log(InfoTag) << "Commands isn't defined. Release call" << iLogW::EndRecord;
			newParticipant->DestroyParticipant();
			return;
		}

		m_calls.Add(new AutotrainerSipCall(getMsgThread(),
			Log(),
			boost::bind(&T::DeleteCall, this, _1),
			newParticipant,
			m_commandsInCall));
	}

	// -------------------------------------------------------------------------------------

	void NObjSipAutotrainer::OnRequestOutgoingParticipant(const ISipAccessPoint *src,
		Utils::SafeRef<IRemoteParticipant> newParticipant,
		const resip::SipMessage &msg,
		const ConversationProfile &conversationProfile)
	{
		if (IsLogActive(InfoTag))
		{
			Log(InfoTag) << "Receive incoming call." << iLogW::EndRecord;
		}

		if (m_commandsInCall.empty())
		{
			Log(InfoTag) << "Commands isn't defined. Release call" << iLogW::EndRecord;
			newParticipant->DestroyParticipant();
			return;
		}

		m_calls.Add(new AutotrainerSipCall(getMsgThread(),
			Log(),
			boost::bind(&T::DeleteCall, this, _1),
			newParticipant,
			m_commandsInCall));
	}

	// -------------------------------------------------------------------------------------

	void NObjSipAutotrainer::SipPoint(const QString &val)
	{
		if (val.isEmpty())
		{
			m_sipAccessPointName = val;
			m_sipAccessPoint.Clear();
			return;
		}

		NObjSipAccessPoint *accessPoint = dynamic_cast<NObjSipAccessPoint*>(
			getDomain().FindFromRoot(Domain::ObjectName(val)));

		if (accessPoint == 0) ThrowRuntimeException("NObjSipUserRegistration named '" + val + "' not found");

		m_sipAccessPointName = val;

		m_sipAccessPoint = accessPoint->AccessPoint();

		m_sipAccessPoint->SetCallReceiver(Name().GetShortName().toStdString(),
			m_selfHost.Create<ISipAccessPointEvents&>(*this)) ;
	}

	// -------------------------------------------------------------------------------------

	void NObjSipAutotrainer::AddCommandToList(QString &commandsStr, AutotrainerSipCall::Commands &commands,
		Commands type, const QString &par)
	{
		AutotrainerSipCall::Command command;
		if (type == Wait)
		{
			bool res = false;
			int  value = par.toInt(&res);
			if (!res || value <0) ThrowRuntimeException("Wrong parameter.");

			command = boost::bind(&AutotrainerSipCall::Wait,	 _1, value);
		}
		else if (type == Dial)
		{
			ESS_ASSERT (!m_sipAccessPoint.IsEmpty());

			resip::Uri destinationUri;
			ConvertStrToUri(par, destinationUri);

			command =
				boost::bind(&AutotrainerSipCall::StartOutgoingCall, _1, m_sipAccessPoint, destinationUri);
		}
		else if (type == RedirectTo)
		{
			resip::Uri destinationUri;
			ConvertStrToUri(par, destinationUri);

			command =
				boost::bind(&AutotrainerSipCall::RedirectTo, _1, destinationUri);
		}
		else if (type == Close)
		{
			int  value = 0;
			if (!par.isEmpty())
			{
				bool res = false;
				if (!res || value <0) ThrowRuntimeException("Wrong parameter.");
			}
			command = boost::bind(&AutotrainerSipCall::Close, _1, value);
		}
		else
		{
			if (!par.isEmpty()) ThrowRuntimeException("Wrong parameter.");

			if (type == Alerting)	command = boost::bind(&AutotrainerSipCall::Alerting,_1);
			if (type == Answer)		command = boost::bind(&AutotrainerSipCall::Answer,	_1);
			if (type == Hold)		command = boost::bind(&AutotrainerSipCall::Hold,	_1);
			if (type == Unhold)		command = boost::bind(&AutotrainerSipCall::Unhold,	_1);
		}

		if (command.empty()) ThrowRuntimeException("Unknown trainer command");

		commandsStr += CommandToString(type, par);

		commands.push_back(command);
	}

	// -------------------------------------------------------------------------------------

	void NObjSipAutotrainer::ConvertStrToUri(const QString &val, resip::Uri &uri)
	{
		std::string error;
		if (!SipUtils::ConvertStrToUri(val, uri, error))
		{
			ThrowRuntimeException(error.c_str());
		}
	}

	// -------------------------------------------------------------------------------------
	
	void NObjSipAutotrainer::DeleteCall(AutotrainerSipCall *call)
	{
		int i = m_calls.Find(call);

		ESS_ASSERT(i != -1);

		PutMsg(this, &T::OnDeleteCall, m_calls.Detach(i));

	}
}
