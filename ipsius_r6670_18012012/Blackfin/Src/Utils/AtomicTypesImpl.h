
#ifndef __ATOMICTYPES__
#define __ATOMICTYPES__

#include "stdafx.h"
#include "Platform/Platform.h"

namespace Utils
{

    class AtomicInt : boost::noncopyable
    {
        volatile int m_value;

    public:

        AtomicInt()
        {
            Set(0);
        }

        explicit AtomicInt(int value)
        {
            Set(value);
        }

        int Get() const
        {
            return m_value;
        }

        void Set(int value)
        {            
            m_value = value; // q_atomic_set_int(&m_value, value);
        }

        void Inc()
        {
            Platform::Atomic::FetchAndAdd(&m_value, 1);
        }

        void Dec()
        {
            Platform::Atomic::FetchAndAdd(&m_value, -1);
        }

        int FetchAndAdd(int val)
        {
            return Platform::Atomic::FetchAndAdd(&m_value, val);
        }

        operator int()
        {
            return m_value;
        }

        // добавить другие q_atomic_xxx операции - ?
        // ...

    };

    // --------------------------------------

    class AtomicBool : boost::noncopyable
    {
        volatile int m_value;

    public:

        AtomicBool()
        {
            Set(false);
        }

        explicit AtomicBool(bool value)
        {
            Set(value);
        }

        bool Get() const
        {
            return m_value ? true : false;
        }

        void Set(bool value)
        {
            m_value = (value) ? true : false; // q_atomic_set_int(&m_value, value ? true : false);
        }

        operator bool() const
        {
            return m_value != 0;
        }

    };

    // --------------------------------------

    template<class T>
    class AtomicPtr : boost::noncopyable
    {
    public:

        // ...

    };

	
}  // namespace Utils


#endif

