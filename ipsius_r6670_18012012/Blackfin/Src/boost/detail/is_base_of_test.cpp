
#include "stdafx.h"

#include "boost/static_assert.hpp"
#include "boost/type_traits/is_base_of.hpp"

namespace
{
    class Base { };
    
    class Derived : public Base {};
    class Derived2 : public Base {};
    class MultiBase : public Derived, public Derived2 {};
    class PrivateBase : private Base {};
    class NonDerived {};

    class VirtualBase 
    {
    public:
        virtual ~VirtualBase(){};
    };
    
    class VirtualDerived : public VirtualBase
    {
    public:
        ~VirtualDerived(){};
    };

    class AbstractBase1
    {
    public:
        AbstractBase1();
        virtual ~AbstractBase1() {};
        AbstractBase1(const AbstractBase1&);
        AbstractBase1& operator=(const AbstractBase1&);
        virtual void Func() = 0;
        virtual void Func2() = 0;
    };
    
    class AbstractBase3 : public AbstractBase1
    {
    public:
        virtual void Func3() = 0;
    };

} // namespace 

// ------------------------------------------

namespace boost
{
    namespace detail
    {
        // commented lines works only with original boost::is_base_of<>
        void is_base_of_test()
        {
            BOOST_STATIC_ASSERT( !(is_base_of<Derived, Base>::value) );
            BOOST_STATIC_ASSERT( (is_base_of<Derived, Derived>::value) );
            BOOST_STATIC_ASSERT( (is_base_of<Base, Base>::value) );
            BOOST_STATIC_ASSERT( (is_base_of<Base, Derived>::value) );
            // BOOST_STATIC_ASSERT( (is_base_of<Base, MultiBase>::value) );
            BOOST_STATIC_ASSERT( (is_base_of<Derived, MultiBase>::value) );
            BOOST_STATIC_ASSERT( (is_base_of<Derived2, MultiBase>::value) );
            // BOOST_STATIC_ASSERT( (is_base_of<Base, PrivateBase>::value) );
            BOOST_STATIC_ASSERT( !(is_base_of<NonDerived, Base>::value) );
            // BOOST_STATIC_ASSERT( !(is_base_of<Base, void>::value) );
            // BOOST_STATIC_ASSERT( !(is_base_of<Base, const void>::value) );
            // BOOST_STATIC_ASSERT( !(is_base_of<void, Derived>::value) );
            // BOOST_STATIC_ASSERT( !(is_base_of<const void, Derived>::value) );
            BOOST_STATIC_ASSERT( (is_base_of<int, int>::value));
            // BOOST_STATIC_ASSERT( !(is_base_of<const int, int>::value) );
            BOOST_STATIC_ASSERT( (is_base_of<VirtualBase, VirtualDerived>::value) );
            BOOST_STATIC_ASSERT( !(is_base_of<VirtualDerived, VirtualBase>::value) );
            BOOST_STATIC_ASSERT( (is_base_of<AbstractBase1, AbstractBase3>::value));
            BOOST_STATIC_ASSERT( !(is_base_of<AbstractBase3, AbstractBase1>::value) );

        }

    } // namesapce detail

} // namespace boost


