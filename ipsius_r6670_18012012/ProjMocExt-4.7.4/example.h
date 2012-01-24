#ifndef __NAMEDOBJECTTESTTYPE__
#define __NAMEDOBJECTTESTTYPE__

#include "Domain/NamedObject.h"

namespace DriTests
{
    class SuperClass {};


    using Domain::NamedObject;
    using Domain::IDomain;
    using Domain::ObjectName;
    
    class NamedObjectTestType : public SuperClass, public NamedObject
    {
        Q_OBJECT;

    public:

        NamedObjectTestType(IDomain *pDomain, const ObjectName &name) : 
          NamedObject(pDomain, name)
        {
            int i = 0; // dummy
			m_property = false;
        }
		
		Q_PROPERTY(bool DirectProperty1 READ m_propertys WRITE m_propertys);		
		Q_PROPERTY(bool DirectProperty READ (m_propertys.field) WRITE (m_propertys.field));

		Q_PROPERTY(bool NFunctionProperty READ (m_propertys.field()) WRITE (m_propertys.field()));
		Q_PROPERTY(bool FunctionProperty READ FunctionProperty WRITE FunctionProperty);
		bool FunctionProperty() const;
		void FunctionProperty(bool val);
	private:
		bool m_property;

    };
    
    class NamedObjectTestType2 : public SuperClass, public NamedObjectTestType
    {
        Q_OBJECT;

    public:

        NamedObjectTestType(IDomain *pDomain, const ObjectName &name) : 
          NamedObject(pDomain, name)
          {
              int i = 0; // dummy
          }

    };
    
}  // namespace DriTests

#endif