#include "stdafx.h"
#include "NameUartPort.h"

namespace 
{
    const std::string devPrefix = "/dev/ttySAC";

} // namespace 

namespace iUart
{
    std::string NameUartPort(int portNumber)
    {
        std::ostringstream out;
        out << devPrefix << portNumber;
        return out.str();
    }

    std::string PlatformInfo()
    {
        std::string ret;
        ret += "Platform \"Arm\" Serial device prefix ";
        ret += devPrefix;
        return ret;
    }
};