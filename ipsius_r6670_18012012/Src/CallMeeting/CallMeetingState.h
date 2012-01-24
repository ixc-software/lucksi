#ifndef _CALL_MEETING_STATE_H_
#define _CALL_MEETING_STATE_H_

#include "stdafx.h" 
#include "Platform/Platform.h" 

namespace CallMeeting
{
	class CallMeetingState : boost::noncopyable
	{
	public:

		struct Rule
		{
			Rule()
			{
				AllowedCall = 0;
				CurrentIncomingCalls = 0;
				CurrentAcceptedCalls = 0;
			}

			bool IsEmpty() const {	return AllowedCall == 0;	}

			QString Country;
			int AllowedCall;
			int CurrentIncomingCalls;
			int CurrentAcceptedCalls;
		};

		typedef QVector<Rule> RuleList;
		
		struct Customer 
		{
			QString Name;

			RuleList Rules;
		};

		typedef QVector<Customer> CustomerList;

		static void Write(const CustomerList &customers);

		static bool Read(CustomerList &customers);

		static QString StoreFilename();
	};

}

#endif
