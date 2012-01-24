#ifndef _RTPPORT_RANGE_LIST_H_
#define _RTPPORT_RANGE_LIST_H_

#include "stdafx.h"
#include "Utils/ManagedList.h"
#include "RtpPortRange.h"

namespace E1App
{
	class RtpPortRangeList : boost::noncopyable,
		public IRtpRangeCreator,
		public IRtpRangeOwner
	{
	public:	
		RtpPortRangeList(int min, int max, int sizeBlock);
		~RtpPortRangeList();
		int MaxSizeBlock() const {	return m_sizeBlock; }
		int AllocatedBlock() const {	return m_allocatedBlock; }
		int MinRtpPort() const	{	return m_min;	}
		int MaxRtpPort() const	{	return m_max;	}

		static void Test();

	// IRtpRangeCreator 
	public:
		boost::shared_ptr<RtpPortRange> AllocRtpPortRange(int countPort);
	// IRtpRangeOwner
	public:
		void FreeRtpRange(RtpPortRange &);
	private:
		int m_sizeBlock;
		int m_min;
		int m_max;
		int m_allocatedBlock;
		Utils::ManagedList<RtpPortRange> m_list;
	};
};
#endif

