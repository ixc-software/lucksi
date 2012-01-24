#ifndef __IS_ITEGRAL__
#define __IS_ITEGRAL__

#include "boost/static_assert.hpp" 

namespace boost
{
        
    template<class T>
    struct is_integral
    {
        typedef bool value_type;
        static const value_type value = false;
    };
    

    #include "detail/template_spec_macro.h"        
    
    	TEMPLATE_SPEC(is_integral, char);
    	TEMPLATE_SPEC(is_integral, unsigned char);
    	TEMPLATE_SPEC(is_integral, short);
    	TEMPLATE_SPEC(is_integral, unsigned short);    	
        TEMPLATE_SPEC(is_integral, int);
        TEMPLATE_SPEC(is_integral, unsigned int);
        TEMPLATE_SPEC(is_integral, long);
        TEMPLATE_SPEC(is_integral, unsigned long);
        TEMPLATE_SPEC(is_integral, long long);
        TEMPLATE_SPEC(is_integral, unsigned long long);
            
    #undef TEMPLATE_SPEC    
        
        
    class is_integral_test
    {
        BOOST_STATIC_ASSERT( is_integral<int>::value );
        BOOST_STATIC_ASSERT( ! is_integral<bool>::value );            
    };
        
    
}  // namespace boost


#endif

