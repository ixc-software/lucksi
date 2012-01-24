#pragma once 

#include <resip/dum/AppDialogSetFactory.hxx>

namespace resip
{
   class AppDialogSet;
   class DialogUsageManager;
   class SipMessage;
}

namespace iSip
{
   class ConversationManager;

	/**
	  This class is used to manage the creation of RemoteParticipant
	  objects.
	*/

	class UserAgentDialogSetFactory : public resip::AppDialogSetFactory
	{
	public:
		UserAgentDialogSetFactory(resip::DialogUsageManager &dum,
			ConversationManager& conversationManager);

		resip::AppDialogSet* createAppDialogSet(resip::DialogUsageManager&,
										 const resip::SipMessage&);    
	private:
		resip::DialogUsageManager &m_dum;
		ConversationManager& m_conversationManager;
	};

}

