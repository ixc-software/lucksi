#pragma once

#include "Utils/IBasicInterface.h"

namespace resip	
{ 
	class SipMessage; 
	class SdpContents;
}

namespace iSip
{
	class ConversationProfile;

	class ISipAccessPointManager : public Utils::IBasicInterface
	{
	public:
		
		Utils::SafeRef<ISipAccessPoint> 
			AccessPoint(const ConversationProfile &conversationProfile);

		void AddAccessPoint(resip::SharedPtr<ConversationProfile> conversationProfile);

		void RemoveAccessPoint(const ConversationProfile &conversationProfile);

		void SetDefIncomingAccessPoint(
			const ConversationProfile &conversationProfile);


	};

}

