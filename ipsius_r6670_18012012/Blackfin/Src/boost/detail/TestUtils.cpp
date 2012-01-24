#include "stdafx.h"
#include "TestUtils.h"

// -------------------------------------------
namespace boost
{
    namespace detail
    {

        std::ostream& DebugStream()
        {
            return std::cout;
        }

    } // namespace detail

} // namespace boost


