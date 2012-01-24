#ifndef __LOGCOMMONIMPLHELPERS__
#define __LOGCOMMONIMPLHELPERS__

#include "Utils/StringUtils.h"

namespace iLogCommon
{

    typedef std::string LogString;
    typedef std::ostringstream LogStringStream;

    struct LogStringConvert
    {

        static LogString To(const std::string &s)
        {
            return s;
        }
        
        static std::string From(const LogString &s)
        {
            return s;
        } 

    };
	
}  // namespace iLogCommon

#endif

