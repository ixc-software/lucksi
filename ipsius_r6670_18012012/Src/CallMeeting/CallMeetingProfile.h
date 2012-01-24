#pragma once 

#include "stdafx.h"
#include "Utils/RangeList.h"
#include "Utils/HostInf.h"

namespace CallMeeting
{

	class CallMeetingProfile : boost::noncopyable
	{
	public:
		CallMeetingProfile();

		bool Validate(std::string &result) const;

		typedef Utils::ValueRange TimeoutRange ;
		TimeoutRange m_tryingTimeout;
		TimeoutRange m_alertingTimeout;
		TimeoutRange m_crossFilterConnectTimeout;

	public:

		bool UseRedirect() const	{ return m_useRedirect; }
		void UseRedirect(bool val)	{ m_useRedirect = val; }
		void WaitConnectionTimeout(int val);
		int WaitConnectionTimeout() const;
		
		void MaxTalkDurationMin(int val);
		int MaxTalkDurationMin() const;

		void ConnectImmediately(bool val);
		bool ConnectImmediately() const;

		void RejectIfDstBusy(bool val);
		bool RejectIfDstBusy() const;
		
		void FakeRtp(const Utils::HostInf &val);		
		const Utils::HostInf &FakeRtp() const;

	private:
		bool m_useRedirect;
		int m_waitConnectionTimeout;
		int m_maxTalkDurationMin;
		bool m_connectImmediately;
		bool m_rejectIfDstBusy;
		Utils::HostInf m_fakeRtp;
	};
};



