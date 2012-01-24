#pragma once

#include "stdafx.h"
#include "ISDN/isdninfr.h"
#include "iLog/LogWrapper.h"

namespace Domain { class DomainClass; };

namespace Dss1ToSip 
{
    class L1DriverForGateProfile;
    class IL1ToHardware;

    class InfraGateDss1 : boost::noncopyable
    {
    public:
        InfraGateDss1(Domain::DomainClass &domain, iLogW::ILogCreatorHelper &log) :
            m_domain(domain),
            m_infraDss1(&m_random, log.getSessionInterface(), log.getLogSessionProfile())
        {}

        Domain::DomainClass &Domain() { return m_domain; } 
        ISDN::IsdnInfra &IsdnInfra() {  return m_infraDss1; }
    private:
        Domain::DomainClass &m_domain; 
        Utils::Random m_random;
        ISDN::IsdnInfraDefault m_infraDss1;
    };
}


