#include "stdafx.h"
#include "NameUartPort.h"

namespace 
{
    const std::string devPrefix = "COM";
} // namespace 

namespace iUart
{
    std::string NameUartPort(int portNumber)
    {
        std::ostringstream out;
        out << "/dev/ttyS" << portNumber;
        return out.str();
    }

    std::string PlatformInfo()
    {
        std::string ret;
        ret += "Platform \"Posix\" Serial device prefix ";
        ret += devPrefix;
        return ret;
    }
};

