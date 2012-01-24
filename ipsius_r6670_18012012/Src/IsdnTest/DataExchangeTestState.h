#ifndef SUBTEST_H
#define SUBTEST_H

// #include "IsdnTestConfig.h"
#include "TestState.h"
#include "LapdTestClass.h"


namespace IsdnTest
{
    

    namespace TestStateImpl
    {
        //  Продумать уничтожение TestState* m_pNextState!!!
        TestState *GetDataExchangeTest(LapdTest& fsm, int maxDuration, TestState* pNextState);

    } // namespace TestStateImpl

} // namespace IsdnTest

#endif
