
#include "stdafx.h"

#include "boost/scoped_ptr.hpp"
#include "lightweight_test.hpp"
#include "boost/static_assert.hpp"

namespace
{
    void f(int &)
    {}
    
    void element_type_test()
    {
        typedef boost::scoped_ptr<int>::element_type T;
        T t;
        f(t);
    }

    // -------------------------------------------------------------

    struct A
    {
        int dummy;
    };

    struct X
    {
        static long instances;
    
        X() { ++instances; }
        virtual ~X() { --instances; }
    
    private:
        X(X const &);
        X & operator= (X const &);
    };

    long X::instances = 0;

    // virtual inheritance stresses the implementation
    struct Y: public A, public virtual X
    {
        Y() {}
        ~Y() {}
    
    private:
    
        Y(Y const &);
        Y & operator= (Y const &);
    };

    // ------------------------------------------------------------

    void constructor_test()
    {
        {
            boost::scoped_ptr<int const> pi(static_cast<int*>(0));
            BOOST_TEST(pi? false: true);
            BOOST_TEST(!pi);
            BOOST_TEST(pi.get() == 0);
        }
    
        {
            boost::scoped_ptr<int volatile> pi(static_cast<int*>(0));
            BOOST_TEST(pi? false: true);
            BOOST_TEST(!pi);
            BOOST_TEST(pi.get() == 0);
        }
    
        {
            boost::scoped_ptr<X> px(static_cast<Y*>(0));
            BOOST_TEST(px? false: true);
            BOOST_TEST(!px);
            BOOST_TEST(px.get() == 0);
        }
    
        {
            boost::scoped_ptr<X const> px(static_cast<Y*>(0));
            BOOST_TEST(px? false: true);
            BOOST_TEST(!px);
            BOOST_TEST(px.get() == 0);
        }
    
        {
            int * p = new int(7);
            boost::scoped_ptr<int> pi(p);
            BOOST_TEST(pi? true: false);
            BOOST_TEST(!!pi);
            BOOST_TEST(pi.get() == p);
            BOOST_TEST(*pi == 7);
        }
    
        {
            int * p = new int(7);
            boost::scoped_ptr<int const> pi(p);
            BOOST_TEST(pi? true: false);
            BOOST_TEST(!!pi);
            BOOST_TEST(pi.get() == p);
            BOOST_TEST(*pi == 7);
        }
    
        {
            X * p = new X;
            boost::scoped_ptr<X> px(p);
            BOOST_TEST(px? true: false);
            BOOST_TEST(!!px);
            BOOST_TEST(px.get() == p);
            BOOST_TEST(X::instances == 1);
        }

        BOOST_TEST(X::instances == 0);
    
        {
            X * p = new X;
            boost::scoped_ptr<X const> px(p);
            BOOST_TEST(px? true: false);
            BOOST_TEST(!!px);
            BOOST_TEST(px.get() == p);
            BOOST_TEST(X::instances == 1);
        }
    
        {
            Y * p = new Y;
            boost::scoped_ptr<X> px(p);
            BOOST_TEST(px? true: false);
            BOOST_TEST(!!px);
            BOOST_TEST(px.get() == p);
            BOOST_TEST(X::instances == 1);
            BOOST_TEST(Y::instances == 1);
        }
    
        BOOST_TEST(X::instances == 0);
        
        {
            Y * p = new Y;
            boost::scoped_ptr<X const> px(p);
            BOOST_TEST(px? true: false);
            BOOST_TEST(!!px);
            BOOST_TEST(px.get() == p);
            BOOST_TEST(X::instances == 1);
            BOOST_TEST(Y::instances == 1);
        }
    
        BOOST_TEST(X::instances == 0);
        BOOST_TEST(Y::instances == 0);
    }

    // ---------------------------------------------------
    
    void access_test()
    {
        {
            boost::scoped_ptr<A> pa;
            BOOST_TEST(pa.get() == 0);
            BOOST_TEST(pa? false: true);
            BOOST_TEST(!pa);
    
            BOOST_TEST(get_pointer(pa) == pa.get());
        }
    
        {
            boost::scoped_ptr<A> pa(static_cast<A*>(0));
            BOOST_TEST(pa.get() == 0);
            BOOST_TEST(pa? false: true);
            BOOST_TEST(!pa);
    
            BOOST_TEST(get_pointer(pa) == pa.get());
        }
  
        {
            A * p = new A;
            boost::scoped_ptr<A> pa(p);
            BOOST_TEST(pa.get() == p);
            BOOST_TEST(pa? true: false);
            BOOST_TEST(!!pa);
            BOOST_TEST(&*pa == pa.get());
            BOOST_TEST(pa.operator ->() == pa.get());
    
            BOOST_TEST(get_pointer(pa) == pa.get());
        }
    }

    // ---------------------------------------------------

    void swap_test()
    {
        {
            boost::scoped_ptr<X> px;
            boost::scoped_ptr<X> px2;

            px.swap(px2);

            BOOST_TEST(px.get() == 0);
            BOOST_TEST(px2.get() == 0);

            using std::swap;
            swap(px, px2);

            BOOST_TEST(px.get() == 0);
            BOOST_TEST(px2.get() == 0);
        }

        {
            X * p = new X;
            boost::scoped_ptr<X> px;
            boost::scoped_ptr<X> px2(p);
            

            px.swap(px2);

            BOOST_TEST(px.get() == p);
            BOOST_TEST(px2.get() == 0);
            
            using std::swap;
            swap(px, px2);

            BOOST_TEST(px.get() == 0);
            BOOST_TEST(px2.get() == p);
        }

        {
            X * p1 = new X;
            X * p2 = new X;
            boost::scoped_ptr<X> px(p1);
            boost::scoped_ptr<X> px2(p2);

            px.swap(px2);

            BOOST_TEST(px.get() == p2);
            BOOST_TEST(px2.get() == p1);
            using std::swap;
            swap(px, px2);

            BOOST_TEST(px.get() == p1);
            BOOST_TEST(px2.get() == p2);
        }
    }

    // ---------------------------------------------------

    void reset_test()
    {
        {
            boost::scoped_ptr<int> pi;

            pi.reset(static_cast<int*>(0));
            BOOST_TEST(pi? false: true);
            BOOST_TEST(!pi);
            BOOST_TEST(pi.get() == 0);

            int * p = new int;
            pi.reset(p);
            BOOST_TEST(pi? true: false);
            BOOST_TEST(!!pi);
            BOOST_TEST(pi.get() == p);

            pi.reset(static_cast<int*>(0));
            BOOST_TEST(pi? false: true);
            BOOST_TEST(!pi);
            BOOST_TEST(pi.get() == 0);
        }

        {
            boost::scoped_ptr<X> px;

            px.reset(static_cast<X*>(0));
            BOOST_TEST(px? false: true);
            BOOST_TEST(!px);
            BOOST_TEST(px.get() == 0);
            BOOST_TEST(X::instances == 0);

            X * p = new X;
            px.reset(p);
            BOOST_TEST(px? true: false);
            BOOST_TEST(!!px);
            BOOST_TEST(px.get() == p);
            BOOST_TEST(X::instances == 1);

            px.reset(static_cast<X*>(0));
            BOOST_TEST(px? false: true);
            BOOST_TEST(!px);
            BOOST_TEST(px.get() == 0);
            BOOST_TEST(X::instances == 0);

            Y * q = new Y;
            px.reset(q);
            BOOST_TEST(px? true: false);
            BOOST_TEST(!!px);
            BOOST_TEST(px.get() == q);
            BOOST_TEST(X::instances == 1);

            px.reset(static_cast<Y*>(0));
            BOOST_TEST(px? false: true);
            BOOST_TEST(!px);
            BOOST_TEST(px.get() == 0);
            BOOST_TEST(X::instances == 0);
        }
    }

} // namespace 

// ---------------------------------------------------

namespace boost
{
    namespace detail
    {
        void scoped_ptr_test(bool silentMode)
        {
            element_type_test();
            constructor_test();
            access_test();
            swap_test();
            reset_test();

            if (silentMode) return;

			int wasErr = boost::report_errors();
			std::cout << "in scoped_ptr_test()." << std::endl ;
        }

    } // namespace detail

} // namespace detail
