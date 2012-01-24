
#include "stdafx.h"

#include "boost/mpl/if.hpp"
#include "boost/type_traits.hpp"
#include "boost/static_assert.hpp"

namespace 
{
    class TypeIfTrue {};
    class TypeIfFalse {};
    
} // namespace 

// ------------------------------------------------------------------------

namespace boost
{
    namespace detail
    {
        void BoostMplIfTest()
        {
            {
                typedef mpl::if_<true, TypeIfTrue, TypeIfFalse>::type GotType;                
                BOOST_STATIC_ASSERT( (boost::is_same<TypeIfTrue, GotType>::value) );
            }

            {
                typedef mpl::if_<false, TypeIfTrue, TypeIfFalse>::type GotType;
                BOOST_STATIC_ASSERT( (boost::is_same<TypeIfFalse, GotType>::value) );
            }
            
            {
                typedef mpl::if_c<true, TypeIfTrue, TypeIfFalse>::type GotType;      
                BOOST_STATIC_ASSERT( (boost::is_same<TypeIfTrue, GotType>::value) );
            }

            {
                typedef mpl::if_c<false, TypeIfTrue, TypeIfFalse>::type GotType;
                BOOST_STATIC_ASSERT( (boost::is_same<TypeIfFalse, GotType>::value) );
            }
        }
        
    } // namespace detail
    
} // namespace boost
