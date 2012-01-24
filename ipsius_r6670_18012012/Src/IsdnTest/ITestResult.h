#ifndef TESTSTERTER_H
#define TESTSTERTER_H

#include "Utils/IBasicInterface.h"

namespace IsdnTest
{                
    class ITestResult : public Utils::IBasicInterface
    {
    public:
        virtual void TestFinished(bool ok, QString info) = 0;
        virtual void PrintInfo(QString info) = 0;
    };                                                                        	        
    
} // namespace IsdnTest

#endif
