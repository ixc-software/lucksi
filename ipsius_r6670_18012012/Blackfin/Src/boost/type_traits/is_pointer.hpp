#ifndef __IS_POINTER__
#define __IS_POINTER__

namespace boost
{
			
    template<class T>
    class is_pointer
    {
        
        static char Fn(...) { return 0; }
        static int Fn(void *p) { return 0; }

        static T MakeT();

    public:
        typedef bool value_type;

        static const value_type value = 
            (sizeof( Fn(MakeT()) ) == sizeof(char)) ? false : true; 

    };

    namespace detail
    {

        class IsPtrTest
        {
            BOOST_STATIC_ASSERT( !is_pointer<int>::value );
            BOOST_STATIC_ASSERT( is_pointer<int*>::value );
            BOOST_STATIC_ASSERT( is_pointer<int**>::value );
        };

    }  // namespace detail
    	
}  // namespace boost


#endif

