#ifndef __FUNCRAII__
#define __FUNCRAII__

#include "Utils/ErrorsSubsystem.h"

namespace Utils
{
    
    class FuncRAII : boost::noncopyable
    {
        typedef boost::function<void ()> Fn;

        Fn m_fn;

        template<class T>
        static void DoPtrReset(boost::scoped_ptr<T> *p)
        {
            p->reset();
        }

    public:
        
        FuncRAII(const Fn &fn) : m_fn(fn)
        {
            ESS_ASSERT( m_fn );
        }
        
        ~FuncRAII()
        {
            m_fn();
        }

    // helpers; also be useful inc, dec op etc. 
    public:

        template<class T>
        static Fn PtrReset(boost::scoped_ptr<T> &p)
        {
            return boost::bind(&FuncRAII::DoPtrReset<T>, &p);
        }
                
    };


    
}  // namespace Utils

#endif