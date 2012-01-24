#include "stdafx.h"

#include "Utils/FullEnumResolver.h"

#include "FullEnumResolverTest.h"

#include "QtEnumResolverExample.h"

// -----------------------------------------------------

namespace
{
    enum Temp { C1 = 1, C2, C3 };

    template<class TEnum>
    static void Verify(TEnum e, const std::string &val)
    {
        std::string res = Utils::EnumResolve(e);
        TUT_ASSERT(res == val);
    }

}

// -----------------------------------------------------

namespace UtilsTests
{

    void RunFullEnumResolverTest()
    {
        Verify(IeConst::RESTRICTED_DIGITAL, "RESTRICTED_DIGITAL");
        Verify(IeConst::KbPs1536, "KbPs1536");
        Verify(C3, "3");
    }


}  // namespace UtilsTest

