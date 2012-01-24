#include "stdafx.h"
#include "LogOut.h"

namespace IsdnTest
{

    LogOut::LogOut(bool console, const std::string file)
        : m_separator(67, '='),
          m_screen(console),
          m_file( file.size() == 0 ? false : true ),
          m_realTimeConsole(true)
          
    {
        if(m_file) m_fileOut.open(file.c_str(), std::ios::out);
        
        Log(m_separator);
        Log("\n");
    }

    LogOut::~LogOut()
    {
        if (!m_realTimeConsole)
            std::cout << m_screenOut.str();

        Log("\n");
        Log(m_separator);
    }

    void LogOut::Log(const std::string &in) // override
    {
        if(m_screen)
        {
            if (m_realTimeConsole)
                std::cout << in << std::endl;
            else
                m_screenOut << in << std::endl;
        }
        if(m_file) m_fileOut << in << std::endl;
    }

} // namespace IsdnTest

