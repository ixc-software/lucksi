#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "g711tests.h"
#include "g711codec.h"
#include "g711gen.h"
#include "get_power.h"

using namespace iDSP;

// -------------------------------------

namespace
{

    //// DecodeALow() vs alaw2linear()
    //void TestDecodeA(bool print)
    //{
    //    using namespace std;

    //    for(int i = 0; i < 0x100; ++i)
    //    {
    //        int val0 = DecodeALow(i) * 8;  // ?!
    //        int val1 = alaw2linear(i);

    //        if (print) cout << val0 << " " << val1 << endl;

    //        TUT_ASSERT(val0 == val1);
    //    }

    //    if (print) cout << endl;
    //}

    //// DecodeMLow() vs ulaw2linear()
    //void TestDecodeU(bool print)
    //{
    //    using namespace std;

    //    for(int i = 0; i < 0x100; ++i)
    //    {
    //        int val0 = DecodeMLow(i) * 4;  // ?
    //        int val1 = ulaw2linear(i);

    //        if (print) cout << val0 << " " << val1 << endl;

    //        TUT_ASSERT(val0 == val1);
    //    }

    //    if (print) cout << endl;
    //}

    //// ULaw_to_ALaw() vs ulaw2alaw()
    //void TestCrossCode()
    //{

    //    for(int i = 0; i < 0x100; ++i)
    //    {
    //        int val0 = ULaw_to_ALaw(i);
    //        int val1 = ulaw2alaw(i);

    //        TUT_ASSERT(val0 == val1);
    //    }

    //}

    void PrintMinMax(bool aLow)
    {
        int min = 0;
        int max = 0;

        for(int i = 0; i < 0x100; ++i)
        {
            int val = aLow ? alaw2linear(i) : ulaw2linear(i);

            if (i == 0)
            {
                min = val;
                max = val;
                continue;
            }

            if (val > max) max = val;
            if (val < min) min = val;
        }

        std::cout << (aLow ? "a-low " : "u-low ") << min << " " << max << std::endl;
    }

}  // namespace


// -------------------------------------

namespace iDSP
{
	
	void G711RunTests()
	{
        // old codec test
        //TestDecodeA(false);   // OK with (val * 8)
        // TestDecodeU(false);  // работает НЕ правильно, поэтому мощность считал через кодинг в A-закон

        // TestCrossCode();  // OK

        // get power test
        TUT_ASSERT( iDSP::GP_Test() );

        // generators test
        iDSP::GeneratorsTest();
	}
	
	void G711PrintMinMaxValues()
	{
        PrintMinMax(true);
        PrintMinMax(false);
	}

	
}  // namespace G711

