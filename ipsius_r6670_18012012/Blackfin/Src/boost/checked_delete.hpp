
#ifndef BOOST_CHECKED_DELETE_HPP_INCLUDED
#define BOOST_CHECKED_DELETE_HPP_INCLUDED

#include "static_assert.hpp"

namespace boost
{
    template<class T> inline void checked_delete(T* x)
    {
        // intentionally complex - simplification causes regressions
        typedef char type_must_be_complete[ sizeof(T)? 1: -1 ];
        (void) sizeof(type_must_be_complete);
        delete x;
    }

	// ---------------------------------------------------
    
    template<class T> inline void checked_array_delete(T * x)
    {
        typedef char type_must_be_complete[ sizeof(T)? 1: -1 ];
        (void) sizeof(type_must_be_complete);
        delete [] x;
    }

	// ---------------------------------------------------
    
    template<class T> struct checked_deleter
    {
        typedef void result_type;
        typedef T* argument_type;
    
        void operator()(T* x) const
        {
            // boost:: disables ADL
            boost::checked_delete(x);
        }
    };

	// ---------------------------------------------------
    
    template<class T> struct checked_array_deleter
    {
        typedef void result_type;
        typedef T * argument_type;
    
        void operator()(T * x) const
        {
            boost::checked_array_delete(x);
        }
    };
    
} // namespace boost


#endif  // #ifndef BOOST_CHECKED_DELETE_HPP_INCLUDED
