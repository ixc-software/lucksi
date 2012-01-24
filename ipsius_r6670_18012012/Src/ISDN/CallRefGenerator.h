#ifndef CALLREFGENERATOR_H
#define CALLREFGENERATOR_H

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"

#include "IL3Calls.h"
#include "CallRef.h"

namespace ISDN
{
    // Генератор метки соединения, назначаемой исходящим вызовам
    class CallRefGenerator : boost::noncopyable
    {
    public:

        CallRefGenerator(dword maxValue, IL3Calls& ICalls) : m_maxCallRef(maxValue, maxValue),
            m_lastCallRef(1, maxValue), m_ICalls(ICalls) {}

        CallRef GenCallRef(bool global = false)
        {   
            if (global)
            {
                CallRef globalCref(m_maxCallRef);
                globalCref.ReplaceValue(0);
                return globalCref;
            }

            while(true)
            {
                int attemptCount = 0;
                ESS_ASSERT( ++attemptCount < 1024 );
                m_lastCallRef.IncrValue();
                if (m_lastCallRef.GetValue() >= m_maxCallRef.GetValue() ) m_lastCallRef.ReplaceValue(1);
                if ( !m_ICalls.Find(m_lastCallRef) ) break;
            }

            return m_lastCallRef;
        }

    private:

        const CallRef m_maxCallRef;
        CallRef m_lastCallRef;
        IL3Calls& m_ICalls;

    };
} // ISDN

#endif

