#pragma once

#include "Platform/PlatformFiber.h"
#include "Utils/ErrorsSubsystem.h"

namespace iCoro
{

    class Coro : boost::noncopyable
    {
        typedef boost::function<void ()> BodyFn;

        enum State
        {
            stInitial,
            stRun,
            stExitRequest,
            stCompleted,
        };

        ESS_TYPEDEF(AbortException);

        BodyFn m_body;
        boost::scoped_ptr<Platform::Fiber> m_fiber;
        State m_state;
        bool m_assertCompleted;
        

        void Body() // fiber context
        {
            m_state = stRun;

            try
            {
                m_body();
            }
            catch(const AbortException &e)
            {
                ESS_ASSERT(m_state == stExitRequest);
            }
            catch(const std::exception &e)
            {
                ESS_UNEXPECTED_EXCEPTION(e);
            }

            // complete
            m_state = stCompleted;
            m_fiber->Return();
        }

    public:

        Coro(const BodyFn &fn = BodyFn(), bool assertCompleted = true) : 
          m_state(stInitial),
          m_assertCompleted(assertCompleted)
        {
            if (fn) SetBody(fn);
        }

        ~Coro()
        {
            if (m_state == stRun)
            {
                if (m_assertCompleted) ESS_HALT("Corotine alive!");
                Abort();
            }
        }

        void SetBody(const BodyFn &fn)
        {
            ESS_ASSERT( fn );
            ESS_ASSERT( m_state == stInitial && !m_body);

            m_body = fn;
        }

        void Abort()
        {
            if (m_state != stRun) return;

            m_state = stExitRequest;
            m_fiber->Run();
        }

        bool Completed() const
        {
            return (m_state == stCompleted);
        }

        void Run()
        {
            if (m_state == stInitial)
            {
                ESS_ASSERT(m_body);
                ESS_ASSERT(m_fiber == 0);

                m_fiber.reset( 
                    new Platform::Fiber( boost::bind(&Coro::Body, this) )
                    );
            }

            ESS_ASSERT( m_state == stInitial || m_state == stRun );

            m_fiber->Run();

            if (m_state == stCompleted)
            {
                m_fiber.reset();
            }
        }

        void Return() // fiber context
        {
            ESS_ASSERT(m_fiber != 0);

            m_fiber->Return();

            // here we are after next Run()
            if (m_state == stExitRequest)
            {
                ESS_THROW(AbortException);
            }
        }

    };
    
    
}  // namespace iCoro

