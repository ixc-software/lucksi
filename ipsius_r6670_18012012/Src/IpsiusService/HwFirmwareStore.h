#ifndef __HWFIRMWARESTORE__
#define __HWFIRMWARESTORE__

#include "stdafx.h"
#include "Utils/StringList.h"
#include "BfBootCore/BroadcastMsg.h"

namespace IpsiusService
{
    using boost::shared_ptr;
    class HwFirmware;

    // ---------------------------------------------------------------
    
    // хранилище прошивок для плат
    class HwFirmwareStore : boost::noncopyable
    {
        Utils::StringList m_badItems;
        typedef std::list<shared_ptr<HwFirmware> >  ItemList;
        ItemList m_items;
        const QString m_path;

    public:        
        HwFirmwareStore(const QString &path);
        void ListFirmware(Utils::StringList &sl, bool withBadItems) const;                                
        shared_ptr<HwFirmware> UpdateRequest(const BfBootCore::BroadcastMsg &msg, bool rollbackMode); //const;

    private:
        void Synchronize(); // can throw;
        bool IsContain(const QFileInfo& info);        
    };
    
}  // namespace IpsiusService

#endif

