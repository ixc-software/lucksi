#ifndef PROFILEHOLDER_H
#define PROFILEHOLDER_H

#include "NObjCommonBfTaskProfile.h"

namespace BfBootDRI
{   
    
    class ProfileHolder 
    {        
        Domain::NamedObject& m_owner;        
        Utils::SafeRef<NObjCommonBfTaskProfile> m_ref;        
    public:        

        ProfileHolder(Domain::NamedObject& owner) : m_owner(owner)
        {}

        bool Init(QString objName, std::string* pErr = 0)
        {           
            NObjCommonBfTaskProfile* pObj = m_owner.getDomain().FindFromRoot<NObjCommonBfTaskProfile>(objName);
            if (pObj == 0) 
            {
                if (pErr) *pErr = objName.append(" is unknown objectName").toStdString();
                return false;
            }            
            pObj->LockChanges(m_owner);     
            m_ref = pObj;
            return true;
        }       

        Utils::SafeRef<NObjCommonBfTaskProfile>& operator->()
        {            
            return m_ref;
        }                   
    };
} // namespace BfBootDRI

#endif
