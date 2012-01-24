#include "stdafx.h"
#include "CallMeetingProfile.h"

namespace CallMeeting
{
	// -------------------------------------------------------------------------------
	// class CallMeetingProfile

	CallMeetingProfile::CallMeetingProfile() : 
		m_tryingTimeout("5000-7000"),
		m_alertingTimeout("7000-25000"),
		m_crossFilterConnectTimeout("0-10000")
	{
		m_useRedirect = false;
		m_waitConnectionTimeout = 60000;
		m_maxTalkDurationMin = 0;
		m_connectImmediately = false;
		m_rejectIfDstBusy = false;
	}

	// -------------------------------------------------------------------------------

	bool CallMeetingProfile::Validate(std::string &result) const
	{
		if(WaitConnectionTimeout() == 0) 
		{
			result = "Wait connection timeout can't be zero.";
			return false;
		}
			
		if(m_tryingTimeout.To() + m_alertingTimeout.To() > m_waitConnectionTimeout) 
		{
			result = "Error: TryingTimeout + AlertingTimeout > WaitConnectionTimeout";
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingProfile::WaitConnectionTimeout(int val)
	{
		m_waitConnectionTimeout = val;
	}

	// -------------------------------------------------------------------------------

	int CallMeetingProfile::WaitConnectionTimeout() const
	{
		return m_waitConnectionTimeout;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingProfile::MaxTalkDurationMin(int val)
	{
		m_maxTalkDurationMin = val;
	}

	// -------------------------------------------------------------------------------

	int CallMeetingProfile::MaxTalkDurationMin() const
	{
		return m_maxTalkDurationMin;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingProfile::ConnectImmediately(bool val)
	{
		m_connectImmediately = val;
	}

	// -------------------------------------------------------------------------------

	bool CallMeetingProfile::ConnectImmediately() const
	{
		return m_connectImmediately;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingProfile::RejectIfDstBusy(bool val)
	{
		m_rejectIfDstBusy = val;
	}

	// -------------------------------------------------------------------------------

	bool CallMeetingProfile::RejectIfDstBusy() const
	{
		return m_rejectIfDstBusy;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingProfile::FakeRtp(const Utils::HostInf &val)
	{
		m_fakeRtp = val;
	}

	// -------------------------------------------------------------------------------

	const Utils::HostInf &CallMeetingProfile::FakeRtp() const
	{
		return m_fakeRtp;
	}


};





