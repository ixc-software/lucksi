#include "stdafx.h"
#include "RegSpanTestsGroup.h"

#include "TestGoertzelDetect.h"
#include "TestSupertoneDetect.h"
#include "TestDtmf.h"

namespace 
{
    using namespace TestFw;

    class SpanTestsGroup : public TestGroup<SpanTestsGroup>
    {

        SpanTestsGroup(ILauncher &launcher) 
            : TestGroup<SpanTestsGroup>(launcher, "Span")             
        {            
            Add("GoertzelDetector test", &iDSP::TestGoertzelDetect, true);
            Add("SupertoneDetector test", &iDSP::TestSupertoneDetect, true);
            Add("DtmfGenerator & DtmfDetect test", &iDSP::TestDtmf, true);
        }

    public:

        static void AddToLauncher(ILauncher &launcher)
        {
            new SpanTestsGroup(launcher);  // free at launcher
        }
    };
} // namespace 

namespace iDSP
{
    void RegSpanTestsGroup(TestFw::ILauncher &launcher)
    {
        SpanTestsGroup::AddToLauncher(launcher);
    }
} // namespace iDSP
