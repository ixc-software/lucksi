#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "ImgDescriptor.h"

namespace
{
    const std::string& Sep()
    {
        static std::string sep(", ");
        return sep;
    }

    template<class T>
    bool StringTo( T& val, const std::string& str )
    {
        std::istringstream ss;
        ss.str(str);
        ss >> val;
        return !ss.fail();
    }
}

namespace BfBootCore
{

    std::string ImgDescriptor::ToString() const
    {
        std::ostringstream ss;
        ss << Id << Sep() << Size << Sep() << Name;

        return ss.str();
    }

    bool ImgDescriptor::setFromString( const std::string& str )
    {
        typedef std::string::size_type Idx;
        Idx  sep1 = str.find_first_of(Sep());
        Idx  sep2 = str.find_first_of(Sep(), sep1 + Sep().size());
        if (sep1 == std::string::npos) return false;
        if (sep2 == std::string::npos) return false;

        ImgDescriptor tmp;
        if ( !StringTo(tmp.Id, str.substr(0, sep1)) ) return false;
        if ( !StringTo(tmp.Size, str.substr(sep1 + Sep().size(), sep2 - sep1)) ) return false;
        tmp.Name = str.substr(sep2 + Sep().size());
        *this = tmp;
        return true;
    }

    bool ImgDescriptor::operator==( const ImgDescriptor& other ) const
    {
        return (Id == other.Id && Size == other.Size && Name == other.Name);
    }

    bool ImgDescriptor::operator!=( const ImgDescriptor& other ) const
    {
        return !operator ==(other);
    }

    
} // namespace BfBootCore

