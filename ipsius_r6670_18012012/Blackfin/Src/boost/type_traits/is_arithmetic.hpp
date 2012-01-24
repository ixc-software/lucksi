#ifndef __IS_ARITHMETIC__
#define __IS_ARITHMETIC__

#include "is_floating_point.hpp"
#include "is_integral.hpp"

namespace boost
{
    
    template<class T>
    struct is_arithmetic
    {
        typedef bool value_type;
        static const value_type value = is_floating_point<T>::value || is_integral<T>::value;        
    };
    
    struct is_arithmetic_test
    {
        BOOST_STATIC_ASSERT( is_arithmetic<int>::value );
        BOOST_STATIC_ASSERT( is_arithmetic<float>::value );
        BOOST_STATIC_ASSERT( ! is_arithmetic<bool>::value );                        
    };
        
}  // namespace boost

#endif

