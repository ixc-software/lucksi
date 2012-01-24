#include "stdafx.h"
#include "Utils/QtEnumResolver.h"
#include "QtEnumResolverExample.h"

// -----------------------------------------------------

namespace UtilsTests
{

    void TestQtEnumResolver()
    {
        Utils::QtEnumResolver<IeConst, IeConst::TransRate> resolver;

        std::string val(resolver.Resolve(IeConst::KbPs1536));
        TUT_ASSERT(val.compare("KbPs1536") == 0);

		
		TUT_ASSERT(resolver.Resolve("KbPs1536") == IeConst::KbPs1536);

		bool resultInd = false;
		TUT_ASSERT(resolver.Resolve("KbPs1536", &resultInd) == IeConst::KbPs1536 &&
			resultInd);
		
		resolver.Resolve("WRONG_VALUE_15361222", &resultInd);
		TUT_ASSERT(!resultInd);
    }
    
    
}  // namespace UtilsTests

