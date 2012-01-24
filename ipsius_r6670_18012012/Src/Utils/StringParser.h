#ifndef __STRINGPARSER__
#define __STRINGPARSER__

#include "Utils/ManagedList.h"

namespace Utils
{
    void StringParser(const std::string& inStr, 
                      const std::string& inSeparator, 
                      Utils::ManagedList<std::string>& outData,
                      bool removeEmpty = false); 

} // namespace Utils 

#endif
