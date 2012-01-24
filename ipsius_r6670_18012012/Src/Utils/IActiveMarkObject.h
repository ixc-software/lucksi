#ifndef __IACTIVEMARKOBJECT__

#define __IACTIVEMARKOBJECT__

#include "stdafx.h"
#include "AtomicTypes.h"
#include "ErrorsSubsystem.h"

namespace Utils
{
	
    class IActiveMarkObject
    {
        AtomicInt m_mark;
        static AtomicInt Counter;

        static int GetMarker()
        {
            while (true)
            {
                Counter.Inc();
                int i = Counter;
                if (i) return i;
            }
        }

    protected:

        void AssertActive() const
        {
            TUT_ASSERT(m_mark.Get());
        }

        IActiveMarkObject() : m_mark(GetMarker())
        {        
        }

        virtual ~IActiveMarkObject()
        {
            AssertActive();
            m_mark.Set(0);
        }

    };
	
}  // namespace Utils


#endif
