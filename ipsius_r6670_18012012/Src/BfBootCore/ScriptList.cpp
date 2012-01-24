#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "ScriptList.h"

namespace BfBootCore
{
    int ScriptList::FindByName( const std::string& name ) const
    {
        for(int i = 0; i < m_list.size(); ++i)
        {
            if(name == m_list.at(i).Name()) return i;
        }
        return NotFound;
    }

    // ------------------------------------------------------------------------------------

    bool ScriptList::Add( const NamedScript& newScript )
    {
        if (FindByName(newScript.Name()) != NotFound) return false;
        m_list.push_back(newScript);
        return true;
    }

    // ------------------------------------------------------------------------------------

    const NamedScript& ScriptList::operator[]( int i ) const
    {
        ESS_ASSERT(i < m_list.size());
        return m_list.at(i);
    }

    // ------------------------------------------------------------------------------------

    int ScriptList::Size() const
    {
        return m_list.size();
    }

    // ------------------------------------------------------------------------------------

    bool ScriptList::operator==( const ScriptList& rhs ) const
    {
        return m_list == rhs.m_list;
    }

    // ------------------------------------------------------------------------------------

    bool ScriptList::SetFrom( std::vector<NamedScript> &list )
    {
        // todo validate duplicate

        m_list.swap(list);
        return true;
    }

    // ------------------------------------------------------------------------------------

    const std::vector<NamedScript>& ScriptList::AsVector() const
    {
        return m_list;
    }

    bool ScriptList::operator!=( const ScriptList& rhs ) const
    {
        return !operator == (rhs);
    }
} // namespace BfBootCore

