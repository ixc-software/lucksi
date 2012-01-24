#ifndef NOBJDSSTRACEOPTION_H
#define NOBJDSSTRACEOPTION_H

#include "DssTraceOption.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

namespace ISDN
{
    class NObjDssTraceOption : public Domain::NamedObject,
        public DRI::INonCreatable,
        public DssTraceOption      
    {
        Q_OBJECT
    public:
        NObjDssTraceOption(Domain::NamedObject *pParent, const Domain::ObjectName &name)
            : Domain::NamedObject(&pParent->getDomain(), name, pParent)
        {
            m_traceOn = false;

            m_traceIeContent = true;
            m_traceIeList = true;
            m_traceDataExcangeL3 = true;

            m_traceDataExcangeL3Call = true;
        }

        Q_PROPERTY(bool Enabled READ m_traceOn WRITE m_traceOn);
        Q_PROPERTY(bool IeContent READ m_traceIeContent WRITE m_traceIeContent);
        Q_PROPERTY(bool IeList READ m_traceIeList WRITE m_traceIeList);
        Q_PROPERTY(bool Data READ m_traceDataExcangeL3 WRITE m_traceDataExcangeL3);
        Q_PROPERTY(bool Calls READ m_traceDataExcangeL3Call WRITE m_traceDataExcangeL3Call);  
        
    };

} // namespace ISDN

#endif
