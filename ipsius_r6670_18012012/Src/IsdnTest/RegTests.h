#ifndef REGTESTS_H
#define REGTESTS_H

#include "TestCollector.h"
#include "RealScenaries.h" // DssTests
#include "NObjLapdTest.h"
#include "ReActivationL3Test.h"

namespace IsdnTest
{
    //class TestCollector;

    static void RegTests(TestCollector& coll)
    {
        RegDssTests(coll);
        RegLapdTests(coll);
        RegDssActivationTests(coll);
    }
} // namespace IsdnTest

#endif
