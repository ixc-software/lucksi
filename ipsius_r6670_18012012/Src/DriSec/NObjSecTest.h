#ifndef __NOBJSECTEST__
#define __NOBJSECTEST__

#include "Domain/NamedObject.h"

namespace DriSec
{
    
    class NObjSecTestBase : public Domain::NamedObject
    {
        Q_OBJECT;

        int m_value;

    public:

        NObjSecTestBase(Domain::IDomain *pDomain, Domain::ObjectName fullName) : 
          Domain::NamedObject(pDomain, fullName),
          m_value(0)
        {
            AllowUserChildrenAdd(true);
        }

        Q_INVOKABLE void Inc() { ++m_value; }
        Q_INVOKABLE void Dec() { --m_value; }
        Q_PROPERTY(int Value READ m_value WRITE m_value);
    };

    // --------------------------------------------------------

    class NObjSecTestA : public NObjSecTestBase
    {
        Q_OBJECT;
    public:
        NObjSecTestA(Domain::IDomain *pDomain, Domain::ObjectName fullName) : NObjSecTestBase(pDomain, fullName) {}
    };

    class NObjSecTestB : public NObjSecTestBase
    {
        Q_OBJECT;
    public:
        NObjSecTestB(Domain::IDomain *pDomain, Domain::ObjectName fullName) : NObjSecTestBase(pDomain, fullName) {}
    };
           
}  // namespace DriSec

#endif