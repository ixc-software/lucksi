#ifndef NOBJREACTIVATIONL3TESTSETTINGS_H
#define NOBJREACTIVATIONL3TESTSETTINGS_H

#include "NObjLapdTraceOption.h"
#include "ISDN/NObjDssTraceOption.h"


namespace IsdnTest
{
    using ISDN::NObjDssTraceOption;

    class NObjReActivationL3TestSettings
        : public Domain::NamedObject,
        public DRI::INonCreatable
    {
        Q_OBJECT

    public:
        NObjLapdTraceOption* const m_pL2TraceOption;
        NObjDssTraceOption* const m_pL3TraceOption;
        
        int m_MaxTestTimeout;
        int m_ReActivationInterval;
        int m_ReActivationCount;

        NObjReActivationL3TestSettings(Domain::NamedObject *pParent, const Domain::ObjectName &name)
            : Domain::NamedObject(&pParent->getDomain(), name, pParent),
            m_pL2TraceOption(new NObjLapdTraceOption(this, "L2TraceOption")),
            m_pL3TraceOption(new NObjDssTraceOption(this, "L3TraceOption"))
        {
            m_ReActivationInterval = 100;
            m_ReActivationCount = 10;
            m_MaxTestTimeout = 1500;
        }

        Q_PROPERTY(int ReActivationInterval READ m_ReActivationInterval WRITE m_ReActivationInterval);
        Q_PROPERTY(int ReActivationCount READ m_ReActivationCount WRITE m_ReActivationCount);
        Q_PROPERTY(int MaxTestTimeout READ m_MaxTestTimeout WRITE m_MaxTestTimeout);
    };
} // namespace IsdnTest

#endif
