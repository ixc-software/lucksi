#include "stdafx.h"

#include "Platform/PlatformFiber.h"
#include "Utils/ErrorsSubsystem.h"

#include "PlatformFiberTest.h"

namespace PlatformTests
{
    using boost::ref;
    typedef boost::function<void ()>                VoidFn;
    typedef boost::function<void (VoidFn yield)>    YieldFn;

    class Fiber
    {
        YieldFn m_body;
        Platform::Fiber m_fiber;

        void Body()
        {
            m_body( 
                boost::bind( &Platform::Fiber::Return, &m_fiber ) 
                );
        }

    public:

        Fiber(YieldFn body) : 
          m_body(body), 
          m_fiber( boost::bind(&Fiber::Body, this) )
        {
        }

        void Run()
        {
            m_fiber.Run();
        }

    };

    // ----------------------------------------------------------

    void InnerFiberBody(VoidFn yield, int &i)
    {
        while(true)
        {
            i += 5;
            yield();
        }
    }

    void MainFiberBody(VoidFn yield, int &i)
    {
        int i2 = 0;

        Fiber f( 
            boost::bind( &InnerFiberBody, _1, ref(i2) ) 
            );

        // loop
        while(true)
        {
            // inner
            i2 = i;
            f.Run();
            TUT_ASSERT( i2 == (i + 5) );

            // next
            ++i;
            yield();
        }
    }

    void RunFiberTest()
    {
        int i = 0;

        Fiber f( 
            boost::bind( &MainFiberBody, _1, ref(i) ) 
            );

        f.Run(); TUT_ASSERT(i == 1);
        f.Run(); TUT_ASSERT(i == 2);
        f.Run(); TUT_ASSERT(i == 3);

    }
        
}  // namespace PlatformTests

