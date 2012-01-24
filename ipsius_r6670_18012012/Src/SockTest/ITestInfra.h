#ifndef __ITESTINFRA__
#define __ITESTINFRA__

#include "Utils/IBasicInterface.h"

namespace SockTest
{

    class ITestInfra : public Utils::IBasicInterface
    {
    public:
        virtual void PrintToLog(const std::string &s) = 0;
    };

}  // namespace SockTest


#endif
