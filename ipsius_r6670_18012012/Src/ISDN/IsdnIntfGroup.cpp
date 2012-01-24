#include "stdafx.h"
#include "BDistributionStrategies.h"
#include "IsdnIntfGroup.h"

namespace ISDN
{
    // ------------------------------------------------------------------------------------
    // IDssToGroup

    int IsdnIntfGroup::CountIntf() const
    {
        return m_listIntf.size();
    }

    // ------------------------------------------------------------------------------------

    ISDN::SharedBChansWrapper IsdnIntfGroup::CaptureAny( int count, int interfaceNumber, bool)
    {
        ESS_ASSERT(m_srategy.get() != 0);

        IsdnIntf* pIntf = (interfaceNumber == -1) ? 
            m_srategy->SelectIntf(m_listIntf, count) : Interface(interfaceNumber);
        
        if (pIntf == 0) return SharedBChansWrapper();

        SetBCannels allocated = m_srategy->SelectChannels(*pIntf, count);

        if (allocated.IsEmpty()) return SharedBChansWrapper();

        pIntf->CaptureBChannel(allocated);

        return SharedBChansWrapper( new BChannelsWrapper(*pIntf, allocated) );
    }

    // ------------------------------------------------------------------------------------

    ISDN::SharedBChansWrapper IsdnIntfGroup::Capture( const SetBCannels& inquredSet, 
		int interfaceNumber, bool exclusive, bool isOutgoing)
    {
        ESS_ASSERT(m_srategy.get() != 0);
        ESS_ASSERT(interfaceNumber != -1 || CountIntf() == 1);  

        IsdnIntf* pIntf = (interfaceNumber == -1) ? &m_listIntf.front() : Interface(interfaceNumber);
        if (pIntf == 0) return SharedBChansWrapper();

        //свободны ли запрошенные?
        if (pIntf->getFree().IsContain(inquredSet))
        {
            pIntf->CaptureBChannel(inquredSet);// занятие            
            return SharedBChansWrapper( new BChannelsWrapper(*pIntf, inquredSet) );
        }

        if (exclusive) return SharedBChansWrapper(); // ничем не могу помочь

        // заняты, назначаем другие
        return CaptureAny(inquredSet.Count(), interfaceNumber, isOutgoing);
    }

    // ------------------------------------------------------------------------------------

    void IsdnIntfGroup::AddIntf(int number, const SetBCannels& set)
    {
        ESS_ASSERT(Interface(number) == 0 && "Duplicate number intrface");
        m_listIntf.push_back( IsdnIntf(set, number) );
    }

    // ------------------------------------------------------------------------------------

    void IsdnIntfGroup::RemoveIntf(int number)
    {
        IntfList::iterator i = m_listIntf.begin();
        while(i != m_listIntf.end())
        {
            if (i->NumberInterface() == number) break;

            ++i;
        }
        ESS_ASSERT(i != m_listIntf.end());
        m_listIntf.erase(i);
    }

    // ------------------------------------------------------------------------------------

    void IsdnIntfGroup::RemoveAllIntf()
    {
        m_listIntf.clear();
    }

    // ------------------------------------------------------------------------------------

    IsdnIntf* IsdnIntfGroup::Interface(int number)
    {
        for (IntfList::iterator i = m_listIntf.begin(); i != m_listIntf.end(); ++i)
        {
            if (i->NumberInterface() == number) return &(*i);
        }
        return 0;
    }

    // ------------------------------------------------------------------------------------

    const IsdnIntf* IsdnIntfGroup::Interface(int number) const 
    {
        for (IntfList::const_iterator i = m_listIntf.begin(); i != m_listIntf.end(); ++i)
        {
            if (i->NumberInterface() == number) return &(*i);
        }
        return 0;
    }

    // ------------------------------------------------------------------------------------

    IsdnIntfGroup::IsdnIntfGroup(int number, const SetBCannels &bChannels)
    {
        SetStrategy<OrderB>();
        AddIntf(number, bChannels);    
    }

} // namespace ISDN

