#ifndef SCRIPTLIST_H
#define SCRIPTLIST_H

#include "NamedScript.h"

namespace BfBootCore
{
    // Список именованных скриптов. Для передачи через протокол загрузчика
    class ScriptList
    {            
        std::vector<NamedScript> m_list;
        
    public:

        enum {NotFound = -1};
        int FindByName(const std::string& name) const;
        bool Add(const NamedScript& newScript);
        const NamedScript& operator[](int i) const;
        int Size() const;        
        // for test
        bool operator ==(const ScriptList& rhs) const;
        bool operator !=(const ScriptList& rhs) const;

    // Используется StringToTypeConverter при передаче 
    // через протокол бутера и при сериализации в Config.
    private:        
        friend class StringToTypeConverter;
        bool SetFrom(std::vector<NamedScript> &list);
        const std::vector<NamedScript>& AsVector() const;
        

    };
} // namespace BfBootCore

#endif
