#ifndef BOOST_SCOPED_PTR_HPP_INCLUDED
#define BOOST_SCOPED_PTR_HPP_INCLUDED


#include "checked_delete.hpp"
#include "Utils/ErrorsSubsystem.h"

namespace boost
{
    //  scoped_ptr mimics a built-in pointer except that it guarantees deletion
    //  of the object pointed to, either on destruction of the scoped_ptr or via
    //  an explicit reset(). scoped_ptr is a simple solution for simple needs;
    //  use shared_ptr or std::auto_ptr if your needs are more complex.
    
    template<class T> class scoped_ptr : noncopyable // noncopyable
    {
        typedef scoped_ptr<T> this_type;

        T *m_pT;
    
        void operator==( scoped_ptr const& ) const;
        void operator!=( scoped_ptr const& ) const;
    
    public:
    
        typedef T element_type;
    
        explicit scoped_ptr(T *p = 0): m_pT(p) // never throws
        {
        }

        /*
        explicit scoped_ptr(std::auto_ptr<T> p): m_pT(p.release()) // never throws
        {
        }
        */
    
        ~scoped_ptr() // never throws
        {
            boost::checked_delete(m_pT);
        }
    
        void reset(T *p = 0) // never throws
        {
            ESS_ASSERT( (p == 0) || (p != m_pT) ); // catch self-reset errors
            this_type(p).swap(*this);
        }
    
        T& operator*() const // never throws
        {
            ESS_ASSERT(m_pT != 0);
            return *m_pT;
        }
    
        T* operator->() const // never throws
        {
            ESS_ASSERT(m_pT != 0);
            return m_pT;
        }
    
        T* get() const // never throws
        {
            return m_pT;
        }
    
        // implicit conversion to "bool"
        operator bool() const
        {
            return m_pT != 0;
        }

        /*
        typedef T* (this_type::*unspecified_bool_type)() const;
        
        operator unspecified_bool_type() const // never throws
        {
            return m_pT == 0 ? 0 : &this_type::get;
        }
        */

        /*
        typedef T * this_type::*unspecified_bool_type;
    
        operator unspecified_bool_type() const // never throws
        {
            return m_pT == 0 ? 0 : &this_type::m_pT;
        }
        */

        bool operator!() const // never throws
        {
            return m_pT == 0;
        }
    
        void swap(scoped_ptr &b) // never throws
        {
            T* tmp = b.m_pT;
            b.m_pT = m_pT;
            m_pT = tmp;
        }
    };

    // --------------------------------------------------------------
    
    template<class T> inline void swap(scoped_ptr<T> &a, scoped_ptr<T> &b) // never throws
    {
        a.swap(b);
    }

    // --------------------------------------------------------------
    
    // get_pointer(p) is a generic way to say p.get()
    template<class T> inline T* get_pointer(scoped_ptr<T> const &p)
    {
        return p.get();
    }

    // --------------------------------------------------------------

    // tests
    namespace detail
    {
        void scoped_ptr_test(bool silentMode = true);

    } // namespace detail 

} // namespace boost

#endif // #ifndef BOOST_SCOPED_PTR_HPP_INCLUDED
