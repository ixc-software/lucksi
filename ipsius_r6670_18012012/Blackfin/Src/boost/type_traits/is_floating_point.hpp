#ifndef __IS_FLOATING_POINT__
#define __IS_FLOATING_POINT__

namespace boost
{
    
    template<class T>
    struct is_floating_point
    {
        typedef bool value_type;
        static const value_type value = false;
    };
    

    #include "detail/template_spec_macro.h"        
    
    	TEMPLATE_SPEC(is_floating_point, float);
    	TEMPLATE_SPEC(is_floating_point, double);
    	TEMPLATE_SPEC(is_floating_point, long double);
            
    #undef TEMPLATE_SPEC    
        
        
    class is_floating_point_test
    {
        BOOST_STATIC_ASSERT( is_floating_point<float>::value );
        BOOST_STATIC_ASSERT( ! is_floating_point<bool>::value );            
    };
    
            
}  // namespace boost

#endif

