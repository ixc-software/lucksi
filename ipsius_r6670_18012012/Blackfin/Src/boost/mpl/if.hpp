
#ifndef BOOST_MPL_IF_HPP_INCLUDED
#define BOOST_MPL_IF_HPP_INCLUDED


namespace boost 
{ 
    namespace mpl 
    {
        template<bool C, typename T1, typename T2>
        class  if_c
        {
        public:
            
            typedef T1 type;
        };

        // -------------------------------------
        
        template<typename T1, typename T2>
        class if_c<false, T1, T2>
        {
        public:
            typedef T2 type;
        };

        // -------------------------------------
        
        template< bool C, typename T1, typename T2>
        class if_
        {
            typedef if_c<C, T1, T2> almost_type_;
         
        public:
            typedef typename almost_type_::type type;
        };

    } // namespace mpl

    // -------------------------------------
    
    namespace detail
    {
        // tests
        void BoostMplIfTest();
        
    } // namespace detail
    
} // naemspace boost

#endif // BOOST_MPL_IF_HPP_INCLUDED
