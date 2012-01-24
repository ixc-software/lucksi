#include "stdafx.h"
#include "IsdnIntf.h"

namespace ISDN
{
    void IsdnIntf::FreeBChannel( const SetBCannels& set )
    {
        ESS_ASSERT(m_busy.IsContain(set));
        m_busy.Exclude(set);
        m_free.Include(set);
    }

	// ------------------------------------------------------------------------------------

	SetBCannels IsdnIntf::CaptureBChannel(int count)
	{
		SetBCannels res;
		if(m_free.Count() < count) return res;
		
		res = m_free.HighSlice(count);
		CaptureBChannel(res);
		return res;
	}

    // ------------------------------------------------------------------------------------

    bool IsdnIntf::CaptureBChannel( const SetBCannels& set )
    {
        ESS_ASSERT(m_free.IsContain(set));
        m_free.Exclude(set);
        m_busy.Include(set);
		return true;
    }

    // ------------------------------------------------------------------------------------

    void IsdnIntf::Blocking(const SetBCannels &set)
    {
        ESS_ASSERT(!m_blocking.IsContain(set));
        ESS_ASSERT(m_free.IsContain(set));
        m_free.Exclude(set);
        m_blocking.Include(set);
    }

    // ------------------------------------------------------------------------------------

    void IsdnIntf::Unblocking(const SetBCannels &set)
    {
        ESS_ASSERT(m_blocking.IsContain(set));
        m_blocking.Exclude(set);
        m_free.Include(set);
    }

    // ------------------------------------------------------------------------------------

    const SetBCannels& IsdnIntf::getFree() const
    {
        return m_free;
    }

    // ------------------------------------------------------------------------------------

    SetBCannels& IsdnIntf::getFree()
    {
        return m_free;
    }

	// ------------------------------------------------------------------------------------

	ISDN::IeConstants::UserInfo IsdnIntf::getPayload() const
	{
		return ISDN::IeConstants::G711_PCMU;
	}

    // ------------------------------------------------------------------------------------

    int IsdnIntf::NumberInterface() const
    {
        return m_number;
    }
} // namespace ISDN

