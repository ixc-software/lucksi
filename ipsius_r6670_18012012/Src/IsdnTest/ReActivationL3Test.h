#ifndef REACTIVATIONL3TEST_H
#define REACTIVATIONL3TEST_H

#include "ProfileBase.h"
#include "ISDN/LapdTraceOption.h"
#include "ISDN/DssTraceOption.h"
#include "Domain/DomainClass.h"
#include "NObjReActivationL3TestSettings.h"

namespace IsdnTest
{
    
    class ITestResult;

    // test class
    class ReActivationL3Test : boost::noncopyable
    {
        class Impl;
        boost::scoped_ptr<Impl> m_impl;
    public:
        // for use in TestCollector
        typedef NObjReActivationL3TestSettings TNObjProfile;        

        void AbortTest(){}
        
        ReActivationL3Test(Domain::DomainClass &domain, TNObjProfile &profile, ITestResult& callback);
        ~ReActivationL3Test();

    };

   
} // namespace IsdnTests

#endif
