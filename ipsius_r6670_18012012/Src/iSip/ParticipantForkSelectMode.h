#pragma once

namespace iSip
{
	class ParticipantForkSelectMode
	{
	public:
		enum Value
		{
			Automatic, // create a conversation for each early fork. accept the first fork from which a 200 is received.  automatically kill other forks 
			Manual     // create a conversation for each early fork. let the application dispose of extra forks. ex: app may form conference. 
		};
	};

}

