
#ifndef BOOST_TT_IS_BASE_OF_HPP_INCLUDED
#define BOOST_TT_IS_BASE_OF_HPP_INCLUDED

#include "boost/static_assert.hpp"

namespace boost
{
    namespace detail
    {
        // use 'value' to check classes relationship 
        template<class Base, class Derived>
        class IsBaseOfCheck
        {
        private:
            typedef char SmallType;
            typedef void* BigType;
            static SmallType Check(Base*);
            static BigType Check(...);
            static Derived* MakeDerived();

        protected:
            static const bool isSame = false;
            static const bool isBase = ( sizeof( Check( MakeDerived() ) ) 
                                               == sizeof(SmallType) );
        };
    
        // -----------------------------------
    
        template<class T>
        class IsBaseOfCheck<T, T>
        {
        protected:
            static const bool isSame = true;
            static const bool isBase = true;
        };

        // -----------------------------------

        // do not check that Base-type is complete
        template<class Base, class Derived>
        class is_base_of_for_shared_ptr
            : public detail::IsBaseOfCheck<Base, Derived>
        {
            BOOST_STATIC_ASSERT(sizeof(Derived) != 0);
            
            typedef detail::IsBaseOfCheck<Base, Derived> TypeCheck;
    
        public:
            typedef bool value_type;
            static const value_type value = TypeCheck::isBase;
        };
        
    } // namespace detail

    // --------------------------------------------------

    template<class Base, class Derived>
    class is_base_of 
        : public detail::IsBaseOfCheck<Base, Derived>
    {
        BOOST_STATIC_ASSERT(sizeof(Base) != 0);
        BOOST_STATIC_ASSERT(sizeof(Derived) != 0);
        
        typedef detail::IsBaseOfCheck<Base, Derived> TypeCheck;

    public:
        typedef bool value_type;
        static const value_type value = TypeCheck::isBase;
        // is_base_of<Base, Derived>::type --- is the type true_type. (?)
    };

    // --------------------------------------------------

    // tests
    namespace detail
    {
        void is_base_of_test();

    } // namespace detail


} // namespace boost 


#endif

