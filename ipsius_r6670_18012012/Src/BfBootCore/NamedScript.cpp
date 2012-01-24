#include "stdafx.h"
#include "NamedScript.h"
#include "ConfigLine.h"

namespace BfBootCore
{
    std::string NamedScript::Name() const
    {
        ESS_ASSERT(m_impl);
        return m_impl->Name();
    }

    // ------------------------------------------------------------------------------------

    std::string NamedScript::Value() const
    {
        ESS_ASSERT(m_impl);
        return m_impl->Value();
    }

    // ------------------------------------------------------------------------------------

    NamedScript::NamedScript( const std::string& name, const std::string& body ) 
        : m_impl( new ConfigLine(name, body))
    {
    }

    // ------------------------------------------------------------------------------------

    std::string NamedScript::getRawString() const
    {
        ESS_ASSERT(m_impl);
        return m_impl->getRawString();
    }

    // ------------------------------------------------------------------------------------

    bool NamedScript::Assign( const std::string& raw )
    {
        try
        {
            m_impl.reset(new ConfigLine(raw));
        }
        catch(ConfigLine::ParseError&)
        {
            return false;
        }
        return true;
    }

    // ------------------------------------------------------------------------------------

    bool NamedScript::operator==( const NamedScript& rhs ) const
    {
        // Assert or return false ??
        ESS_ASSERT(m_impl);
        ESS_ASSERT(rhs.m_impl);

        return m_impl->IsEqual(*rhs.m_impl);
    }

    // ------------------------------------------------------------------------------------
   
    bool NamedScript::operator != (const NamedScript& rhs) const 
    { 
        return !operator == (rhs);
    }

    // ------------------------------------------------------------------------------------

    const std::string& NamedScript::NameOfBootScript() // static
    {
        static std::string bootName("BOOT");
        return bootName;
    }

} // namespace BfbootCore

