
#include "stdafx.h"

#include "boost/static_assert.hpp"
#include "boost/type_traits/is_same.hpp"

namespace boost
{
    namespace detail
    {
        // commented lines works only with original boost::is_same<>
        void is_same_test()
        {
            BOOST_STATIC_ASSERT( (is_same<int, int>::value) );
            BOOST_STATIC_ASSERT( !(is_same<int, const int>::value) );
            BOOST_STATIC_ASSERT( !(is_same<int, int&>::value) );
            BOOST_STATIC_ASSERT( !(is_same<const int, int&>::value) );
            BOOST_STATIC_ASSERT( !(is_same<int, const int&>::value) );
            BOOST_STATIC_ASSERT( !(is_same<int*, const int*>::value) );
            BOOST_STATIC_ASSERT( !(is_same<int*, int* const>::value) );
            /*
            BOOST_STATIC_ASSERT( !(is_same<int, int[2]>::value) );
            BOOST_STATIC_ASSERT( !(is_same<int*, int[2]>::value) );
            BOOST_STATIC_ASSERT( !(is_same<int[4], int[2]>::value) );
            */
        }

    } // namespace detail

} // namespace boost










