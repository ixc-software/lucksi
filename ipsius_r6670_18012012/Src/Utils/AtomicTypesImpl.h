
#ifndef __ATOMICTYPES__
#define __ATOMICTYPES__

#include "stdafx.h"
#include "Platform/Platform.h"

namespace Utils
{

    class AtomicInt : boost::noncopyable
    {
        QAtomicInt m_value;

    public:
		explicit AtomicInt(int value = 0) : m_value(value)
        {}

        int Get() const
        {
            return m_value;
        }
        void Set(int value)
        {            
            m_value.testAndSetOrdered(m_value, value); // q_atomic_set_int(&m_value, value);
        }

        void Inc()
        {
            m_value.ref();
        }

        void Dec()
        {
            m_value.deref();
        }

        int FetchAndAdd(int val)
        {
            return m_value.fetchAndAddOrdered(val);
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
		QAtomicInt m_value;
	
    public:

		explicit AtomicBool(bool value = false) : m_value((value) ? 1 : 0)
        {}

        bool Get() const
        {
            return m_value == 1;
        }

        void Set(bool value)
        {
			m_value.testAndSetOrdered(m_value, value); // q_atomic_set_int(&m_value, value);
//            m_value = (value) ? 1 : 0; // q_atomic_set_int(&m_value, value ? true : false);
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

