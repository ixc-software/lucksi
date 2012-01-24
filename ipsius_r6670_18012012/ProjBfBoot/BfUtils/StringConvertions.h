#ifndef __STRINGCONVERTIONS__
#define __STRINGCONVERTIONS__


#include "stdafx.h"


namespace BfUtils
{


    // Converts Value to string via std::stringstream
    template<class T>
    inline std::string ToString(const T &Value)
    {
        std::stringstream streamOut;
        streamOut << Value;
        return streamOut.str();
    }


    // Converts value from string via std::stringstream
    template<class T>
    inline T FromString(const std::string &ToConvert)
    {
        std::stringstream streamIn(ToConvert);
        T ReturnValue = T();
        streamIn >> ReturnValue;
        return ReturnValue;
    }

} // namespace StringConvertions


#endif
