#ifndef INFRATEST_H
#define INFRATEST_H

// #include "IsdnTestConfig.h"
#include "ISDN/isdninfr.h"
#include "LogOut.h"

namespace IsdnTest
{
	typedef ISDN::IsdnInfraDefault IsdnInfraTest;
    //class IsdnInfraTest: public ISDN::IsdnInfraDefault
    //{
    //
    //public:

    //    IsdnInfraTest(Utils::Random* pRnd, LogOut& out):
    //      ISDN::IsdnInfraDefault(pRnd), m_out(out) {}
    //    
    //    void Log(const std::string &in) // override IIsdnLog in IsdnInfraDefault
    //    {
    //        m_out.Log(in);
    //    }

    //private:

    //    LogOut &m_out;

    //};
};

#endif
