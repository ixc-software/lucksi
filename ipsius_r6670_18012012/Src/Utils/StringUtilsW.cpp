#include "stdafx.h"
#include "StringUtilsW.h"
#include "StringUtilsImpl.h"

namespace Utils
{

    std::wstring PadLeft(const std::wstring &s, int length, char c)
    {
        std::wstring res;
        PadString(s, length, true, c, res);
        return res;
    }

    std::wstring PadRight(const std::wstring &s, int length, char c)
    {
        std::wstring res;
        PadString(s, length, false, c, res);        
        return res;
    }

    std::wstring StringToWString( const std::string &s )
    {
        std::wstring res;

        for(int i = 0; i < s.length(); ++i)
        {
            res += s.at(i);
        }

        return res;
    }

    std::string WStringToString( const std::wstring &s )
    {
        std::string res;

        for(int i = 0; i < s.length(); ++i)
        {
            res += s.at(i);
        }

        return res;
    }

}  // namespace Utils


