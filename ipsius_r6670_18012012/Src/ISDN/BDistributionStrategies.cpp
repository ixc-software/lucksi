#include "stdafx.h"
#include "BDistributionStrategies.h"
#include "IsdnIntf.h"

namespace ISDN
{
    IsdnIntf *OrderB::SelectIntf(IntfList &list, int count)
    {
        for (IntfList::iterator i = list.begin(); i != list.end(); ++i)
        {
            if (i->getFree().Count() >= count) return &(*i);
        }
        return 0;
    }

    // ------------------------------------------------------------------------------------

    ISDN::SetBCannels OrderB::SelectChannels( const IsdnIntf& intf, int count ) const
    {
        return intf.getFree().HighSlice(count);
    }

    // ------------------------------------------------------------------------------------   

    IsdnIntf *OrderF::SelectIntf(IntfList &list, int count)
    {
        for (IntfList::iterator i = list.begin(); i != list.end(); ++i)
        {
            if (i->getFree().Count() >= count) return &(*i);
        }
        return 0;
    }

    // ------------------------------------------------------------------------------------

    ISDN::SetBCannels OrderF::SelectChannels( const IsdnIntf& intf, int count ) const
    {
        return intf.getFree().LowSlice(count);
    }
} // namespace ISDN

