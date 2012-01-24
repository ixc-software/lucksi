#ifndef ISDNSTACKSETTINGS_H
#define ISDNSTACKSETTINGS_H

#include "stdafx.h"
#include "L2Profiles.h"
#include "L3Profiles.h"

namespace ISDN
{

    using boost::shared_ptr;

    struct IsdnStackSettings
    {
    
        IsdnStackSettings(const shared_ptr<const L2Profile> L2prof, const shared_ptr<const L3Profile> L3prof)
            : m_L2prof(L2prof),        
            m_L3prof(L3prof)
        {}

        const shared_ptr<const L2Profile> m_L2prof;            
        const shared_ptr<const L3Profile> m_L3prof;
    
    };

} // namespace ISDN

#endif
