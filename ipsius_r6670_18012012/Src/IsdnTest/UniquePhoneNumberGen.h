#ifndef UNIQUEPHONENUMBERGEN_H
#define UNIQUEPHONENUMBERGEN_H

#include "ISDN/DssUpIntf.h" // DssPhoneNumber
#include "stdafx.h"

namespace IsdnTest
{
    using ISDN::DssPhoneNumber;

    // Генератор телефонных номеров
    class UniquePhoneNumberGen : boost::noncopyable
    {
        enum
        {
            CDigit = (sizeof(dword) == 4) ? 10 : ( sizeof(dword) == 8  ?  20 : 0 ),
            CMaxValue = ~dword(0)
        };

        std::string m_lastString;
        dword m_lastNum;

        void GenString()
        {
            BOOST_STATIC_ASSERT(CDigit);
            ESS_ASSERT(m_lastNum < CMaxValue);
            
            std::stringstream ss;
            ss.width(CDigit); ss.fill('0');
            ss << m_lastNum;
            m_lastString = ss.str();
        }

    public:

        UniquePhoneNumberGen()
            : m_lastNum(0) // 0 - reserved
        {}

        DssPhoneNumber Generate()
        {
            if (m_lastNum == CMaxValue)
                m_lastNum = 1; // 0 - reserved
            else 
                ++m_lastNum;

            GenString();

            return DssPhoneNumber(m_lastString);
        }
    };

    

} //IsdnTest

#endif

