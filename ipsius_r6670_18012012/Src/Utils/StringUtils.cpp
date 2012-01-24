#include "stdafx.h"

#include "Platform/Platform.h"

#include "ErrorsSubsystem.h"
#include "StringUtils.h"
#include "StringUtilsImpl.h"

namespace
{

    char CharUpCase(char c)
    {
        if ((c >= 'a') && (c <= 'z')) return c - ('a' - 'A');
        return c;
    }

    char CharLowCase(char c)
    {
        if ((c >= 'A') && (c <= 'Z')) return c + ('a' - 'A');
        return c;
    }

}

// --------------------------------------------------------

namespace Utils
{
    using Platform::byte;

    std::string PadLeft(const std::string &s, int length, char c)
    {
        std::string res;
    	PadString(s, length, true, c, res);
        return res;
    }

    // ------------------------------------------------------

    std::string PadRight(const std::string &s, int length, char c)
    {
        std::string res;
        PadString(s, length, false, c, res);        
        return res;
    }

    // ------------------------------------------------------
    
    std::string TrimBlanks(const std::string &s)
    {
        if (!s.size()) 
            return "";

        size_t begin = s.find_first_not_of(" \n\r\t");
        size_t end = s.find_last_not_of(" \n\r\t");
        if ((begin == std::string::npos) || (end == std::string::npos))
            return "";
        return s.substr(begin, end + 1 - begin);
    }

    // ------------------------------------------------------

    bool StringUpCase(const std::string &in, std::string &out)
    {
        ESS_ASSERT(in.size() == out.size());

        bool ok = true;

        for(int i = 0; i < in.size(); ++i)
        {
            char c = in.at(i);
            if ((byte)c >= 0x80) ok = false;
            out.at(i) = CharUpCase(c);
        }

        return ok;
    }

    // ------------------------------------------------------

    std::string StringUpCase(const std::string &s, bool *pAllCharsOK)
    {
        std::string res = s;
        bool ok = StringUpCase(s, res);
        if (pAllCharsOK != 0) *pAllCharsOK = ok;
        return res;
    }

    // ------------------------------------------------------

    bool StringLowerCase(const std::string &in, std::string &out)
    {
        ESS_ASSERT(in.size() == out.size());

        bool ok = true;

        for(int i = 0; i < in.size(); ++i)
        {
            char c = in.at(i);
            if ((byte)c >= 0x80) ok = false;
            out.at(i) = CharLowCase(c);
        }

        return ok;
    }

    // ------------------------------------------------------

    std::string StringLowerCase(const std::string &s, bool *pAllCharsOK)
    {
        std::string res = s;
        bool ok = StringLowerCase(s, res);
        if (pAllCharsOK != 0) *pAllCharsOK = ok;
        return res;
    }

    // ------------------------------------------------------

    void StringCaseConvTest()
    {
        bool ok;

        // up case
        TUT_ASSERT(StringUpCase("~1@ DooM", &ok) == "~1@ DOOM");
        TUT_ASSERT(ok);

        TUT_ASSERT(StringUpCase("DooM папа Zzz", &ok) == "DOOM папа ZZZ");
        TUT_ASSERT(!ok);

        // lo case
        TUT_ASSERT(StringLowerCase("~1@ DooM", &ok) == "~1@ doom");
        TUT_ASSERT(ok);

        TUT_ASSERT(StringLowerCase("DooM папа Zzz", &ok) == "doom папа zzz");
        TUT_ASSERT(!ok);
    }

    // ------------------------------------------------------

    bool StringStartWith(const std::string &s, const std::string &val)
    {
        if (s.size() < val.size()) return false;

        return (s.substr(0, val.size()) == val);
    }


}  // namespace Utils

