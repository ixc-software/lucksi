#ifndef __THREADCONTEXT__
#define __THREADCONTEXT__

#include "stdafx.h"
#include "Platform/Platform.h"
#include "ErrorsSubsystem.h"

namespace Utils
{

    // Класс, для проверки корректности контекста потока
    class ThreadContext
    {
        bool m_empty;
        Platform::Thread::ThreadID m_handle;
        bool m_assertOnDestroy;

    public:

        explicit ThreadContext(bool assertOnDestroy = true) : m_assertOnDestroy(assertOnDestroy)
        {
            Set();
        }

        ~ThreadContext()
        {
            if (m_assertOnDestroy) Assert();
        }

        bool Current() const
        {
            ESS_ASSERT(!m_empty);
            return ( m_handle == Platform::Thread::GetCurrentThreadID() );
        }

        void Assert() const
        {
            if (m_empty) return;

            ESS_ASSERT( Current() );
        }

        void Set()
        {
            m_handle = Platform::Thread::GetCurrentThreadID();
            m_empty = false;
        }

        void Clear()
        {
            m_empty = true;
        }

    };

}  // namespace Utils


#endif

