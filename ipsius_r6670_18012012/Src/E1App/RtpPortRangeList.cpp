#include "stdafx.h"
#include "RtpPortRangeList.h"

namespace 
{
	int CapacityList(int size, int sizeBlock)
	{
		return size / sizeBlock + ((size%sizeBlock == 0) ? 0 : 1);
	}

	int CapacityList(int min, int max, int sizeBlock)
	{
		ESS_ASSERT(max >= min);
		
		return CapacityList(max - min, sizeBlock);
	}
};
namespace E1App
{
	RtpPortRangeList::RtpPortRangeList(int min, int max, int sizeBlock) :
		m_sizeBlock(sizeBlock),
		m_list(true, CapacityList(min, max, m_sizeBlock), true)
	{
		m_allocatedBlock = 0;
		m_min = min;
		m_max = max;
		int count = CapacityList(min, max, m_sizeBlock);
		int begin = min;
		for(int i = 0; i < count; ++i)
		{
			int end = begin + m_sizeBlock;
			if (end > max) end = max;
			m_list.Add(new RtpPortRange(begin, end - 1));
			begin = end;
		}
	}

	// -----------------------------------------------------------------------

	RtpPortRangeList::~RtpPortRangeList()
	{
		ESS_ASSERT(m_allocatedBlock == 0);
	}

	// -----------------------------------------------------------------------
	// IRtpRangeCreator 

	boost::shared_ptr<RtpPortRange> RtpPortRangeList::AllocRtpPortRange(int countPort)
	{
		ESS_ASSERT(countPort <= m_sizeBlock);

		boost::shared_ptr<RtpPortRange> result;

		for(int i = 0; i < m_list.Size(); ++i)
		{
			if(m_list[i] == 0) continue;
			result.reset(new RtpPortRange(m_list[i]->MinPort(), m_list[i]->MaxPort(), this));
			m_list.Set(i, 0);
			m_allocatedBlock++;
			break;
		}
		return result;
	}

	// -----------------------------------------------------------------------
	// IRtpRangeOwner

	void RtpPortRangeList::FreeRtpRange(RtpPortRange &range)
	{
		ESS_ASSERT(m_allocatedBlock != 0);
		int pos = range.MinPort() - m_min;
		ESS_ASSERT((pos%m_sizeBlock) == 0);
		pos = pos/m_sizeBlock;
		ESS_ASSERT(m_list[pos] == 0);		

		if(pos != 0 && m_list[pos - 1] != 0) ESS_ASSERT(m_list[pos - 1]->MaxPort() + 1== range.MinPort());		
		if(pos != m_list.Size() - 1 && m_list[pos + 1] != 0) ESS_ASSERT(m_list[pos + 1]->MinPort() - 1 == range.MaxPort());
		m_allocatedBlock--;
		
		m_list.Set(pos, new RtpPortRange(range.MinPort(), range.MaxPort()));
	}

	void RtpPortRangeList::Test() // static
	{	
		{
			boost::scoped_ptr<RtpPortRangeList> list(new RtpPortRangeList(0, 0, 10));
			
			ESS_ASSERT(list->AllocRtpPortRange(10) == 0);
		}

		boost::scoped_ptr<RtpPortRangeList> list(new RtpPortRangeList(10, 100, 10));

		boost::shared_ptr<RtpPortRange> range1 = list->AllocRtpPortRange(10);
		boost::shared_ptr<RtpPortRange> range2 = list->AllocRtpPortRange(5);
		boost::shared_ptr<RtpPortRange> range3 = list->AllocRtpPortRange(10);
		range2.reset();
		range3.reset();
		range1.reset();
		range1 = list->AllocRtpPortRange(10);
		range2 = list->AllocRtpPortRange(5);
		range3 = list->AllocRtpPortRange(10);
		range1.reset();
		range3.reset();
		range2.reset();
		range1 = list->AllocRtpPortRange(10);
		range2 = list->AllocRtpPortRange(5);
		range3 = list->AllocRtpPortRange(10);
		range3.reset();
		range2.reset();
		range1.reset();
	}

};


