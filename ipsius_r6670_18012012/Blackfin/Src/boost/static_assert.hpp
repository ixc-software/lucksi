
#ifndef BOOST_STATIC_ASSERT_HPP
#define BOOST_STATIC_ASSERT_HPP


namespace boost
{
    namespace detail
    {
        // tests
        void boost_static_assert_test();

    } // namespace detail
   
} // namespace boost

// ------------------------------------------------

/*
#define BOOST_STATIC_ASSERT(B) ( boost::detail::StaticAssertionFailure< (B) != 0 >() )
*/


namespace boost
{

#ifdef _MSC_VER   // MSVC workaround

    template <bool x> struct STATIC_ASSERTION_FAILURE;
    template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };

    template<int x> struct static_assert_test{};

    #define BOOST_STATIC_ASSERT( B ) \
        typedef \
        boost::static_assert_test< sizeof( boost::STATIC_ASSERTION_FAILURE<(bool)( B )> ) >  \
        boost_static_assert_typedef_

    /*
    #define BOOST_STATIC_ASSERT( expr ) \
    void assert_failed_( int (*arg) [2 / !!(expr) - 1] ) */

#else

    #define BOOST_STATIC_ASSERT( expr ) \
        void assert_failed_( int (*arg) [2 / !!(expr) - 1] [__LINE__] )

#endif



}  // namespace boost



#endif
