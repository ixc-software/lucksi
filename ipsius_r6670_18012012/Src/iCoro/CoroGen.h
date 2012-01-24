#pragma once

#include "Coro.h"

namespace iCoro
{
    ESS_TYPEDEF(GenEnd);
    
    template<class TOut>
    class Gen : boost::noncopyable
    {
    public:

        typedef boost::function<void (TOut)>        YieldFn;
        typedef boost::function<void (YieldFn)>     GenFn;

    private:

        typedef Gen<TOut> T;

        enum State 
        {
            stUndefined,
            stEnded,
            stValueReady,
        };

        GenFn m_fn;
        int m_limit;
        int m_runCount;

        Coro m_coro;

        // value
        TOut m_val;
        State m_state;

        void YieldHandle(TOut val)
        {
            ESS_ASSERT(m_state == stUndefined);

            m_state = stValueReady;
            m_val = val;

            m_coro.Return();
        }

        void Body()
        {
            m_fn( boost::bind(&T::YieldHandle, this, _1) );
        }

    public:

        Gen(const GenFn &fn, int limit = -1) : 
          m_fn(fn), m_limit(limit), m_runCount(0),
          m_coro( boost::bind(&T::Body, this), false ),
          m_val(),
          m_state(stUndefined)
        {
        }

        bool End() // it's not const 'couse it actually peek next value 
        {
            if (m_state == stValueReady) return false;
            if (m_state == stEnded)      return true;

            bool limited = (m_limit > 0) && (m_runCount >= m_limit);
            if (m_coro.Completed() || limited) 
            {
                m_state = stEnded;                    
                return true;
            }

            // try run
            ESS_ASSERT(m_state == stUndefined);
            ++m_runCount;
            m_coro.Run();

            if ( m_coro.Completed() ) 
            {
                m_state = stEnded;                    
                return true;
            }

            ESS_ASSERT(m_state == stValueReady);
            return false;
        }

        TOut Next()
        {
            if ( End() ) ESS_THROW(GenEnd);

            ESS_ASSERT(m_state == stValueReady);
            m_state = stUndefined;
            return m_val;
        }

    };

    
}  // namespace iCoro

