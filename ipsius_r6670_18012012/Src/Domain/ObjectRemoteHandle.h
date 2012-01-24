#ifndef __OBJECTREMOTEHANDLE__

#define __OBJECTREMOTEHANDLE__

#include "stdafx.h"
#include "DomainRPC.h"

namespace Domain
{

    typedef int ObjectDescriptor;
    
    /*
        Дескриптор объекта на удаленной машине 
    */
    class ObjectRemoteHandle
    {
        ObjectDescriptor m_objectID; 
        DomainRPC &m_drpc;

    public:
        ObjectRemoteHandle(const ObjectDescriptor &objectID, DomainRPC &drpc)
            : m_objectID(objectID), m_drpc(drpc)
        {
            // ...
        }

        DomainRPC& GetDRCP()
        {
            return m_drpc;
        }

        ObjectDescriptor getID() const
        {
            return m_objectID;
        }
    };


}  // namespace Domain


#endif
