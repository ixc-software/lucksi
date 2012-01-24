#ifndef IBDISTRIBSTRATEGY_H
#define IBDISTRIBSTRATEGY_H

#include "Utils/IBasicInterface.h"
#include "SetBCannels.h"

namespace ISDN
{
    class IsdnIntf;
    typedef std::list<IsdnIntf> IntfList;

    // Интрефейс стратегии выделения B канала в Dss1
    class IBDistribStrategy : Utils::IBasicInterface
    {                    
    public:
        virtual IsdnIntf* SelectIntf(IntfList &list, int count) = 0;
        virtual SetBCannels SelectChannels(const IsdnIntf& intf, int count ) const = 0;
    };
} // namespace ISDN

#endif
