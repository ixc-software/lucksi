
#include "stdafx.h"
#include "boost/static_assert.hpp"

namespace 
{
    // Namespace scope
    BOOST_STATIC_ASSERT(sizeof(int) >= sizeof(short));
    BOOST_STATIC_ASSERT(sizeof(char) == 1);

    // ----------------------------------------

    // Function (block) scope
    void f()
    {
        BOOST_STATIC_ASSERT(sizeof(int) >= sizeof(short));
        BOOST_STATIC_ASSERT(sizeof(char) == 1);
    }

    // ----------------------------------------
    
    class ClassA
    {
    private:  // can be in private, to avoid namespace pollution

        BOOST_STATIC_ASSERT( (sizeof(int) >= sizeof(short)) );
        BOOST_STATIC_ASSERT(sizeof(char) == 1);

    public:
    
        // Member function scope: provides access to member variables
        int x;
        char c;
        int f()
        {
            BOOST_STATIC_ASSERT(sizeof(x) >= sizeof(short));
            BOOST_STATIC_ASSERT(sizeof(c) == 1);
            return x;
        }
    };

    // ----------------------------------------
    
    // Template class scope
    template <class Int, class Char>
    class ClassB
    {
    private:  // can be in private, to avoid namespace pollution
        BOOST_STATIC_ASSERT(sizeof(Int) > sizeof(char));
    public:
    
        // Template member function scope: provides access to member variables
        Int x;
        Char c;
        template <class Int2, class Char2>
        void f(Int2 , Char2 )
        {
            BOOST_STATIC_ASSERT(sizeof(Int) == sizeof(Int2));
            BOOST_STATIC_ASSERT(sizeof(Char) == sizeof(Char2));
        }
    };

    void TestClassB() // BOOST_STATIC_ASSERTs are not triggerred until instantiated
    {
        ClassB<int, char> z;
        // ClassB<int, int> bad; // will not compile
        int i = 3;
        char ch = 'a';
        z.f(i, ch);
        // z.f(i, i); // should not compile
    }


} // namespace 

// ----------------------------------------

namespace boost
{
    namespace detail
    {
        void boost_static_assert_test()
        { 
            TestClassB();
        }

    } // namespace detail

} // namespace boost

