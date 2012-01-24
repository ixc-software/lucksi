#ifndef __ICORELOGTEST__
#define __ICORELOGTEST__

#include "Utils/IBasicInterface.h"

namespace iCoreTests
{
	
    class ILoggable : public Utils::IBasicInterface
    {
    public:
        virtual void Log(const std::string &s) = 0;
    };

    class Logger : public ILoggable
    {
        bool m_silenceMode;

    public:

        Logger(bool silenceMode) : m_silenceMode(silenceMode) {}

        // ILoggable
        void Log(const std::string &s)
        {
            if (m_silenceMode) return;
            std::cout << s << std::endl;
        }

    };
		
}  // namespace iCoreTests


#endif



