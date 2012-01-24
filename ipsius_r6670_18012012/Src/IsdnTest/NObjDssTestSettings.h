#ifndef NOBJDSSTESTSETTINGS_H
#define NOBJDSSTESTSETTINGS_H

#include "DRI/INonCreatable.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

#include "NObjLapdTraceOption.h"
#include "ISDN/NObjDssTraceOption.h"

namespace IsdnTest
{
    using ISDN::NObjDssTraceOption;

    class NObjDssTestSettings
        : public Domain::NamedObject,
        public DRI::INonCreatable
    {
        Q_OBJECT   
    public: //?
        NObjLapdTraceOption* const m_pL2TraceOption;
        NObjDssTraceOption* const m_pL3TraceOption;
        bool m_TraceTest;
        int m_MaxTestTimeout;

    public:

        NObjDssTestSettings(Domain::NamedObject *pParent, const Domain::ObjectName &name)
            : Domain::NamedObject(&pParent->getDomain(), name, pParent),
            m_pL2TraceOption(new NObjLapdTraceOption(this, "L2TraceOption")),
            m_pL3TraceOption(new NObjDssTraceOption(this, "L3TraceOption")),
            m_TraceTest(false),
            m_MaxTestTimeout(50 * 1000)
        {            
        }

        Q_PROPERTY(bool TraceTest READ m_TraceTest WRITE m_TraceTest);
        Q_PROPERTY(int MaxTestTimeout READ m_MaxTestTimeout WRITE m_MaxTestTimeout);
    };
} // namespace IsdnTest

#endif
