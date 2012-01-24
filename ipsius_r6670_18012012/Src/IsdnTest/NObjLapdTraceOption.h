#ifndef NOBJLAPDTRACEOPTION_H
#define NOBJLAPDTRACEOPTION_H

#include "ISDN/LapdTraceOption.h"

#include "DRI/INonCreatable.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

namespace IsdnTest
{
    class NObjLapdTraceOption : 
        public Domain::NamedObject,
        public DRI::INonCreatable,
        public ISDN::LapdTraceOption
    {
        Q_OBJECT        
        
    public:

        NObjLapdTraceOption(Domain::NamedObject *pParent, const Domain::ObjectName &name)
            : Domain::NamedObject(&pParent->getDomain(), name, pParent)
        {}

        Q_PROPERTY(bool On WRITE m_traceOn READ m_traceOn);
        Q_PROPERTY(bool PacketRaw WRITE m_traceRaw READ m_traceRaw);
    };
} // namespace IsdnTest

#endif
