#ifndef __OBJECTLINKERROR__
#define __OBJECTLINKERROR__

#include "stdafx.h"
#include "Domain/ObjectName.h"

namespace ObjLink
{
    // ключ-идентификатор для ObjectLink
    class IObjectLinkOwnerNotify;
    typedef IObjectLinkOwnerNotify ILinkKeyID;
	
	// базовый класс ошибок для ObjectLink
	class ObjectLinkError
	{
        ILinkKeyID *m_pLinkID;

        virtual void WriteErrorInfo(std::stringstream &ss)
        {
            ss << "ObjectLinkError undefined for type " << typeid(*this).name();
        }

    public:
        ObjectLinkError(ILinkKeyID *pLinkID) : m_pLinkID(pLinkID) {}
        virtual ~ObjectLinkError() {}

        std::string ToString()
        {
            std::stringstream ss;
            WriteErrorInfo(ss);
            return ss.str();
        }

        bool Equal(const ILinkKeyID &keyID) const
        {
            return (m_pLinkID == &keyID);
        }

	};	

    // -------------------------------------------------------------
    // -------------------------------------------------------------

    class LocalNamedObjectError : public ObjectLinkError
    {
        Domain::FullObjectName m_name;

    public:
        LocalNamedObjectError(ILinkKeyID *pLinkID, const Domain::FullObjectName &name)
            : ObjectLinkError(pLinkID), m_name(name) 
        {            
        }
    };

    // -------------------------------------------------------------

    class ErrorLocalNamedObjectNotFound : public LocalNamedObjectError
    {
    public:
        ErrorLocalNamedObjectNotFound(ILinkKeyID *pLinkID, const Domain::FullObjectName &name)
            : LocalNamedObjectError(pLinkID, name) 
        {            
        }

    };

    // -------------------------------------------------------------

    class ErrorLocalNamedObjectCastToHost : public LocalNamedObjectError
    {
    public:
        ErrorLocalNamedObjectCastToHost(ILinkKeyID *pLinkID, const Domain::FullObjectName &name)
            : LocalNamedObjectError(pLinkID, name) 
        {            
        }

    };

    // -------------------------------------------------------------

    class ErrorLocalNamedObjectCastToInterface : public LocalNamedObjectError
    {
    public:
        ErrorLocalNamedObjectCastToInterface(ILinkKeyID *pLinkID, const Domain::FullObjectName &name)
            : LocalNamedObjectError(pLinkID, name)  
        {
        }

    };

    // -------------------------------------------------------------
    // -------------------------------------------------------------

    class ErrorLocalCallLost : public ObjectLinkError
    {
    public:
        ErrorLocalCallLost(ILinkKeyID *pLinkID) : ObjectLinkError(pLinkID) {}

    };
	
}  // namespace ObjLink

#endif

