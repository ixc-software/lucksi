#ifndef BDISTRIBUTIONSTRATEGYES_H
#define BDISTRIBUTIONSTRATEGYES_H

#include "stdafx.h"
#include "IBDistribStrategy.h"

namespace ISDN
{        
    class IsdnIntf;

    // real strategy
    class OrderB : boost::noncopyable, public IBDistribStrategy
    {
    // IBDistribStrategy impl:
    private:
        IsdnIntf *SelectIntf(IntfList &list, int count);
        SetBCannels SelectChannels(const IsdnIntf& intf, int count ) const;
    };

    class OrderF : boost::noncopyable, public IBDistribStrategy
    {
    // IBDistribStrategy impl:
    private:
        IsdnIntf *SelectIntf(IntfList &list, int count);
        SetBCannels SelectChannels(const IsdnIntf& intf, int count ) const;
    };

} // namespace ISDN

#endif
