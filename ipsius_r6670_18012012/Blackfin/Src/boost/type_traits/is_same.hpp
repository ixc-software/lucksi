
#ifndef BOOST_TT_IS_SAME_HPP_INCLUDED
#define BOOST_TT_IS_SAME_HPP_INCLUDED

namespace boost
{
    namespace detail
    {
        // use 'value' to check classes relationship 
        template<class Base, class Derived>
        class IsSameCheck
        {
        private:
            typedef char SmallType;
            typedef void* BigType;
            static SmallType Check(Base);
            static BigType Check(...);
            static Derived MakeDerived();

        protected:
            static const bool isSame = false;
            static const bool isBase = ( sizeof( Check( MakeDerived() ) ) 
                                               == sizeof(SmallType) );
        };
    
        // -----------------------------------
    
        template<class T>
        class IsSameCheck<T, T>
        {
        protected:
            static const bool isSame = true;
            static const bool isBase = true;
        };

    } // namespace detail

    // --------------------------------------------------

    // value == true, if ClassA is same as ClassB
    template<class ClassA, class ClassB>
    class is_same 
        : public detail::IsSameCheck<ClassA, ClassB>
    {
        typedef detail::IsSameCheck<ClassA, ClassB> TypeCheck;

    public:
        typedef bool value_type;
        static const value_type value = (TypeCheck::isBase && TypeCheck::isSame);
        // is_same<ClassA, ClassB>::type --- is the type true_type (?)
    };

    // --------------------------------------------------

    namespace detail
    {
        void is_same_test();

    } // namespace detail

} // namespace boost

#endif
