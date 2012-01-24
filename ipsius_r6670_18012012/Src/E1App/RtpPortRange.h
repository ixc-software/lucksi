#ifndef _RTPPORT_RANGE_H_
#define _RTPPORT_RANGE_H_

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"

namespace E1App
{
	class RtpPortRange;

	class IRtpRangeOwner : public Utils::IBasicInterface
	{
	public:
		virtual void FreeRtpRange(RtpPortRange &) = 0;
	};

	// -------------------------------------------------------------------------------

	class IRtpRangeCreator : public Utils::IBasicInterface
	{
	public:
		virtual boost::shared_ptr<RtpPortRange> AllocRtpPortRange(int countPort) = 0;
	};

	// -------------------------------------------------------------------------------

	class RtpPortRange : boost::noncopyable
	{
	public:
		RtpPortRange(int min = 0, int max = 0, IRtpRangeOwner *owner = 0) : 
            m_owner(owner), m_min(min), m_max(max)
		{
			ESS_ASSERT(m_min <= m_max);
		}
		~RtpPortRange()
		{
			if(m_owner) m_owner->FreeRtpRange(*this);
		}
	int MinPort()const {	return m_min; }
		int MaxPort()const {	return m_max; }
	private:
		IRtpRangeOwner *m_owner;
		int m_min;
		int m_max;
	};
};

#endif
