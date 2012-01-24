#ifndef __LOGCOMMONIMPLHELPERS__
#define __LOGCOMMONIMPLHELPERS__

#include "Utils/StringUtilsW.h"

namespace iLogCommon
{

    typedef std::wstring LogString;
    typedef std::wostringstream LogStringStream;

    struct LogStringConvert
    {

        static LogString To(const std::string &s)
        {
            return Utils::StringToWString(s);
        }
        
        static std::string From(const LogString &s)
        {
            return Utils::WStringToString(s);
        } 

    };
	
}  // namespace iLogCommon

#endif

