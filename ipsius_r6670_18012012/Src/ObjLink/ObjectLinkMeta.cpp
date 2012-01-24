#include "stdafx.h"
#include "ObjectLinkMeta.h"

// ------------------------------------------------------

namespace ObjLink
{

    namespace
    {
        typedef std::map<std::string, IObjectLinkInterfaceMeta*> Map;

        Map& GMap()
        {
            static Map *p = 0;  // create once, never deleted
            if (p == 0) p = new Map;
            return *p;
        }

    }

    // ----------------------------------------------------

    void ObjectLinkInterfacesFactory::RegisterMeta(IObjectLinkInterfaceMeta *pMeta)
    {
        ESS_ASSERT(pMeta);

        std::string typeName = pMeta->TypeName();

        // check for correctness typeName
        // ...

        // dublicates check
        ESS_ASSERT(GMap().find(typeName) == GMap().end());

        // register
        GMap()[typeName] = pMeta;
    }

    // ----------------------------------------------------

    const IObjectLinkInterfaceMeta* ObjectLinkInterfacesFactory::FindMeta(const std::string &name)
    {
        Map::const_iterator i = GMap().find(name); 
        if (i == GMap().end()) return 0;
        return (*i).second;
    }

	
}  // namespace ObjLink


