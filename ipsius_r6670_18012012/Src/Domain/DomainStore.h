#ifndef __DOMAINSTORE__
#define __DOMAINSTORE__

#include "Utils/IVirtualDestroy.h"

namespace Domain
{
    
    // Domain related object store
    class DomainStore
    {
        typedef std::string StoreName;
        typedef std::map<StoreName, Utils::IVirtualDestroy*> Map;

        Map m_map;

    public:

        DomainStore()
        {
        }

        ~DomainStore()
        {
            Map::iterator i = m_map.begin();

            while(i != m_map.end())
            {
                delete i->second;

                ++i;
            }
        }

        bool StoreExists(const StoreName &storeName) const 
        {
            return (m_map.find(storeName) != m_map.end());
        }

        // pStore free in ~DomainStore
        void Register(const StoreName &storeName, Utils::IVirtualDestroy *pStore)
        {
            ESS_ASSERT( !StoreExists(storeName) );
            ESS_ASSERT(pStore != 0);

            m_map[storeName] = pStore;
        }

        template<class T>
        T& Get(const StoreName &storeName)
        {
            Map::iterator i = m_map.find(storeName);
            ESS_ASSERT(i != m_map.end());

            T *p = dynamic_cast<T*>(i->second);
            ESS_ASSERT(p != 0);

            return *p;
        }

    };
    
}  // namespace Domain

#endif
