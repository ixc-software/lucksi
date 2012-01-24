
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#define BOOST_SHARED_PTR_HPP_INCLUDED

#include "type_traits.hpp"
#include "static_assert.hpp"
#include "detail/SharedCounter.h"

#include "Utils/ErrorsSubsystem.h"

/*
	Define SHARED_PTR_DISABLE_THREAD_SAFETY if your system do not support 
    atomic operations or if you do not want to use atomic operations in 
    boost::shared_ptr<>.
*/

namespace boost
{
    namespace detail
    {
        /*
        struct static_cast_tag {};
        struct const_cast_tag {};
        struct dynamic_cast_tag {};
        struct polymorphic_cast_tag {};
        */

        template<class T> struct shared_ptr_traits
        {
            typedef T& reference;
        };
        
        template<> struct shared_ptr_traits<void>
        {
            typedef void reference;
        };

        template<> struct shared_ptr_traits<void const>
        {
            typedef void reference;
        };
        
        template<> struct shared_ptr_traits<void volatile>
        {
            typedef void reference;
        };
        
        template<> struct shared_ptr_traits<void const volatile>
        {
            typedef void reference;
        };
    
    } // namespace detail

    // ------------------------------------------

    //  shared_ptr
    //
    //  An enhanced relative of scoped_ptr with reference counted copy semantics.
    //  The object pointed to is deleted when the last shared_ptr pointing to it
    //  is destroyed or reset.
    
    template<class T>
    class shared_ptr
    {
        typedef shared_ptr<T> this_type;
        typedef detail::SharedCounter Counter;

        template<class> friend class shared_ptr;

        T *m_pT; // do not delete this
        Counter m_counter; // when call copy constructor it incs counter 

    public:

        typedef T element_type;
        typedef T value_type;
        typedef T* pointer;
        typedef typename boost::detail::shared_ptr_traits<T>::reference reference;

        
        shared_ptr() // never throws
        : m_pT(0)             // counter == 0
        {}

        template<class Y> explicit shared_ptr(Y *p)
        : m_pT(p), m_counter(p)
        {
            // p must be convertible to T *.
            // BOOST_STATIC_ASSERT( (is_base_of<T, Y>::value) );
            BOOST_STATIC_ASSERT( (detail::is_base_of_for_shared_ptr<T, Y>::value) );
        }

        /*
        template<class Y, class D> shared_ptr(Y *p, D d);
        template<class Y, class D, class A> shared_ptr(Y *p, D d, A a);
        */

        ~shared_ptr() // never throws
        {
            // do nothing
        }
        
        shared_ptr(shared_ptr const &r) // never throws
        : m_pT(r.m_pT), m_counter(r.m_counter)
        {
        }

        template<class Y> 
        shared_ptr(shared_ptr<Y> const &r) // never throws
        : m_pT(r.m_pT), m_counter(r.m_counter)
        {
        }
        
        template<class Y> 
        shared_ptr(shared_ptr<Y> const &r, T *p) // never throws
        : m_pT(p), m_counter(r.m_counter)
        {
        }

        /*
        template<class Y> explicit shared_ptr(weak_ptr<Y> const &r);
        template<class Y> explicit shared_ptr(std::auto_ptr<Y> &r);
        */

        shared_ptr& operator=(shared_ptr const &r) // never throws  
        {
            m_pT = r.m_pT;
            m_counter = r.m_counter;
            
            return *this;
        }

        template<class Y> 
        shared_ptr& operator=(shared_ptr<Y> const &r) // never throws
        {
            m_pT = r.m_pT;
            m_counter = r.m_counter;

            return *this;
        }
        
        /*
        template<class Y> shared_ptr& operator=(std::auto_ptr<Y> &r);
        */

        void reset() // never throws
        {
            this_type().swap(*this);
        }
        
        template<class Y> 
        void reset(Y *p)
        {
            ESS_ASSERT(p == 0 || p != m_pT); // catch self-reset errors
            this_type(p).swap(*this);
        }

        /*
        template<class Y, class D> void reset(Y *p, D d);
        template<class Y, class D, class A> void reset(Y *p, D d, A a);
        */

        template<class Y> void reset(shared_ptr<Y> const &r, T *p) // never throws
        {
            this_type(r, p).swap( *this );
        }

        reference operator*() const // never throws
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

        bool unique() const // never throws
        {
            return m_counter.Unique();
        }

        long use_count() const // never throws
        {
            return m_counter.UseCount();
        }

        void swap(shared_ptr &b) // never throws
        {
            std::swap(m_pT, b.m_pT);
            m_counter.Swap(b.m_counter);
        }

        // implicit conversion to "bool"
        operator bool() const
        {
            return m_pT != 0;
        }


        /*
        static void unspecified_bool( this_type*** )
        {
        }
    
        typedef void (*unspecified_bool_type)( this_type*** );
    
        operator unspecified_bool_type() const // never throws
        {
            return m_pT == 0 ? 0: unspecified_bool;
        }
        */
            

        /*
        typedef T * (this_type::*unspecified_bool_type)() const;
    
        operator unspecified_bool_type() const // never throws
        {
            return px == 0? 0: &this_type::get;
        }
        */

        /*
        typedef T * this_type::*unspecified_bool_type;
    
        operator unspecified_bool_type() const // never throws
        {
            return px == 0? 0: &this_type::px;
        }
    
        */
    
        // operator! is redundant, but some compilers need it
        bool operator! () const // never throws
        {
            return m_pT == 0;
        }

        template<class Y> bool _internal_less(shared_ptr<Y> const &rhs) const
        {
            return m_counter < rhs.m_counter;
        }
    };

    // --------------------------------------------------------------
    
    template<class T, class U>
    inline bool operator==(shared_ptr<T> const &a, shared_ptr<U> const &b) // never throws
    {
        return a.get() == b.get();
    }

    // --------------------------------------------------------------

    template<class T, class U>
    inline bool operator!=(shared_ptr<T> const &a, shared_ptr<U> const &b) // never throws
    {
        return a.get() != b.get();
    }

    // --------------------------------------------------------------

    template<class T, class U>
    inline bool operator<(shared_ptr<T> const &a, shared_ptr<U> const &b) // never throws
    {
        return a._internal_less(b);
    }

    // --------------------------------------------------------------

    template<class T> 
    inline void swap(shared_ptr<T> &a, shared_ptr<T> &b) // never throws
    {
        a.swap(b);
    }

    // --------------------------------------------------------------

    template<class T> 
    inline T* get_pointer(shared_ptr<T> const &p) // never throws
    {
        return p.get();
    }

    // --------------------------------------------------------------

    /*
    template<class T, class U>
    inline shared_ptr<T> static_pointer_cast(shared_ptr<U> const &r); // never throws
  
    template<class T, class U>
    inline shared_ptr<T> const_pointer_cast(shared_ptr<U> const &r); // never throws
  
    template<class T, class U>
    inline shared_ptr<T> dynamic_pointer_cast(shared_ptr<U> const &r); // never throws
  
    template<class E, class T, class Y>
    inline std::basic_ostream<E, T>& operator<< (std::basic_ostream<E, T> &os, shared_ptr<Y> const &p);
  
    template<class D, class T>
    inline D* get_deleter(shared_ptr<T> const &p);
    */

    // --------------------------------------------------------------
    
    // tests
    namespace detail
    {
        void shared_ptr_test(bool silentMode = true);

    } // namespace detail

} // namespace boost


#endif
