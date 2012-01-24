#ifndef LOGTOFILE
#define LOGTOFILE

#include "stdafx.h"
// #include "IsdnTestConfig.h"
#include "ISDN/IIsdnLog.h"

namespace IsdnTest
{
    // имплементац€ ISDN::IIsdnLog, вывод трасировки в консоль и в файл
    class LogOut: public ISDN::IIsdnLog, boost::noncopyable
    {
    public:

        LogOut(bool console, const std::string file = "");

        ~LogOut();

        void Log(const std::string &in); // override

    private:

        std::string m_separator;

        bool m_screen;
        bool m_file;
        std::ofstream m_fileOut;
        std::stringstream m_screenOut;
        bool m_realTimeConsole;
    };

} // namespace IsdnTest

#endif
