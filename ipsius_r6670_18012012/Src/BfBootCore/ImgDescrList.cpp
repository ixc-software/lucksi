#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "ImgDescrList.h"

namespace BfBootCore
{

    int ImgDescrList::Size() const
    {
        return m_list.size();
    }

    const ImgDescriptor& ImgDescrList::operator[]( int i ) const
    {
        ESS_ASSERT(i < m_list.size());
        return m_list.at(i);
    }

    void ImgDescrList::Add( const ImgDescriptor& descr )
    {
        m_list.push_back(descr);
    }

    const ImgDescriptor& ImgDescrList::Back() const
    {
        return m_list.back();
    }

    void ImgDescrList::DeleteLast()
    {
        m_list.pop_back();
    }

    bool ImgDescrList::operator==( const ImgDescrList& rhs ) const
    {
        return m_list == rhs.m_list;
    }

    bool ImgDescrList::operator!=( const ImgDescrList& rhs ) const
    {
        return !operator == (rhs);
    }

    void ImgDescrList::SetFrom( std::vector<ImgDescriptor> &list )
    {
        m_list.swap(list);
    }

    const std::vector<ImgDescriptor>& ImgDescrList::AsVector() const
    {
        return m_list;
    }

    std::string ImgDescrList::ToString() const
    {
        std::ostringstream oss;

        for(int i = 0; i < m_list.size(); ++i)
        {
            const ImgDescriptor &item = m_list.at(i);
            oss << item.Name << " (" << item.Size << "); ";
        }

        return oss.str();
    }

    void ImgDescrList::Clear()
    {
        m_list.clear();
    }
} // namespace BfBootCore

