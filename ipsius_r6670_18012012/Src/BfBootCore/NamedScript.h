#ifndef NAMEDSCRIPT_H
#define NAMEDSCRIPT_H

#include "stdafx.h"
#include "IHwFirmwareScript.h"


namespace BfBootCore
{
    class ConfigLine;

    class NamedScript : public IHwFirmwareScript
    {
        boost::shared_ptr<ConfigLine> m_impl; 

    // IHwFirmwareScript impl
    public:
        std::string Name() const;
        std::string Value() const;

        NamedScript()
        {}
        NamedScript(const std::string& name, const std::string& body);

        // используется при передаче через протокол бутера
        std::string getRawString() const;
        bool Assign(const std::string& raw);

        // used in tests
        bool operator == (const NamedScript& rhs) const;
        bool operator != (const NamedScript& rhs) const;

        // имя зарезервированное для основного (загрузочного скрипта)
        static const std::string& NameOfBootScript();
    };

   
    
} // namespace BfBootCore

#endif
