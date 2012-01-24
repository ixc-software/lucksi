#ifndef __EVENTTIME__
#define __EVENTTIME__

#include "iVDK/CriticalRegion.h"
#include "iVDK/VdkUtils.h"

namespace BfDev
{
	
	class EventTime
	{
        int m_enter;
        int m_max;
        int m_counter;

	public:

        EventTime()
        {
            m_enter = 0;
            m_max = 0;
            m_counter = 0;
        }

        void Enter()
        {
            int currTime = iVDK::GetSystemTickCount();

            iVDK::CriticalRegion::Enter();

            m_enter = currTime;

            iVDK::CriticalRegion::Leave();
        }

        void Leave()
        {
            int currTime = iVDK::GetSystemTickCount();

            iVDK::CriticalRegion::Enter();

            int diff = currTime - m_enter;
            m_enter = 0;

            if (diff > m_max) m_max = diff;

            m_counter++;

            iVDK::CriticalRegion::Leave();
        }

        int Check()
        {
            int currTime = iVDK::GetSystemTickCount();

            bool inEvent = (m_enter != 0);
            int diff = currTime - m_enter;

            return diff;
        }

	};
	
}  // namespace BfDev

#endif
