#pragma once 

#include "ConversationManager.h"
#include <rutil/SharedPtr.hxx>
#include <resip/dum/AppDialogSet.hxx>

namespace resip
{
	class DialogUsageManager;
	class SipMessage;
	class UserProfile;
}

namespace iSip
{
	class DefaultDialogSet : public resip::AppDialogSet
	{
	public:  

		DefaultDialogSet(resip::DialogUsageManager &dum,
			ConversationManager &conversationManager) :
			resip::AppDialogSet(dum),
			m_conversationManager(conversationManager)
		{
		}
	
	private:

		resip::SharedPtr<resip::UserProfile> selectUASUserProfile(const resip::SipMessage& msg)
		{
			return m_conversationManager.GetIncomingConversationProfile(msg)->UserProfilePtr();
		}

	private:       
		ConversationManager &m_conversationManager;
	};

}



