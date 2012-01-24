#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"

#define _WIN32_WINNT 0x0400
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "PlatformFiber.h"

namespace Platform
{
    typedef boost::function<void ()> BodyFn;

    void MainFiberId() {}

    struct Fiber::FiberImpl
    {
        static const int CDefaultStackSize = 256 * 1024;

        BodyFn m_bodyFn;
        LPVOID m_handle;
        LPVOID m_fiberToReturn;

        static void __stdcall FiberFunc(LPVOID lpParameter)
        {
            ESS_ASSERT(lpParameter);

            FiberImpl *p = static_cast<FiberImpl*>(lpParameter);

            try
            {
                p->m_bodyFn();
                ESS_HALT("Never goes here!");
            }
            catch(const std::exception &e)
            {
                ESS_UNEXPECTED_EXCEPTION(e);
            }
        }

        static void MainThreadInit()
        {            
            PVOID pData = GetCurrentFiber();
            if (pData == (void*)0x1E00)  // magic
            {
                LPVOID h = ConvertThreadToFiber( &MainFiberId );
                ESS_ASSERT(h != 0);
                ESS_ASSERT( GetFiberData() == &MainFiberId );
            }
        }

    public:

        FiberImpl(const BodyFn &fn, int stackSize) : m_bodyFn(fn), m_fiberToReturn(0)
        {
            ESS_ASSERT(fn);
            if (stackSize <= 0) stackSize = CDefaultStackSize;

            MainThreadInit();

            // create fiber
            m_handle = CreateFiber(stackSize, &FiberImpl::FiberFunc, this);
            ESS_ASSERT(m_handle != 0);
        }

        ~FiberImpl()
        {
            DeleteFiber(m_handle);
        }

        void Run()
        {
            ESS_ASSERT(m_fiberToReturn == 0);

            m_fiberToReturn = GetCurrentFiber();
            ESS_ASSERT(m_fiberToReturn != 0);
            ESS_ASSERT(m_fiberToReturn != m_handle);

            SwitchToFiber(m_handle);
        }

        void Return()
        {
            ESS_ASSERT( GetCurrentFiber() == m_handle );

            ESS_ASSERT( m_fiberToReturn != 0);
            LPVOID ret = m_fiberToReturn;
            m_fiberToReturn = 0;
            SwitchToFiber(ret);
        }

    };

    // -------------------------------------------------------

    Fiber::Fiber(const BodyFn &fn, int stackSize)
    {
        m_body.reset( new FiberImpl(fn, stackSize) );
    }

    Fiber::~Fiber()
    {
        // nothing
    }

    void Fiber::Run()
    {
        m_body->Run();
    }

    void Fiber::Return()
    {
        m_body->Return();
    }

}  // namespace Platform


