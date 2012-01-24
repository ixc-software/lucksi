#include "stdafx.h"
#include "iSip/undeferrno.h"

#include "UserAgentDialogSetFactory.h"
#include "RemoteParticipantDialogSet.h"
#include "ConversationManager.h"
#include "DefaultDialogSet.h"

namespace iSip
{
	using namespace resip;

	UserAgentDialogSetFactory::UserAgentDialogSetFactory(resip::DialogUsageManager &dum,
		ConversationManager& conversationManager) :
		m_dum(dum),
		m_conversationManager(conversationManager)
	{
	}

	// -------------------------------------------------------------------------------------

	AppDialogSet* UserAgentDialogSetFactory::createAppDialogSet(DialogUsageManager &dum,
		const SipMessage &msg)
	{
		if(msg.method() ==  INVITE)
			return new RemoteParticipantDialogSet(dum, m_conversationManager, m_conversationManager.LogCreator());
			
		return new DefaultDialogSet(dum, m_conversationManager);
	}

}
