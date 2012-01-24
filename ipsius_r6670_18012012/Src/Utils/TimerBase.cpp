
#include "stdafx.h"
#include "TimerBase.h"

namespace Utils
{


	// ----------------------------------------------------------

	/*
	TimerBase::TimerBase(dword interval, bool autoreset)
	{
		Init();
		Set(interval, autoreset);
	} */

	// ----------------------------------------------------------

	void TimerBase::Init()
	{
		Stop();
	}


	// ----------------------------------------------------------

	TimerBase::TimerBase(void)
	{
		Init();
	}

	// ----------------------------------------------------------

	dword TimerBase::StartVal() const
	{
		return m_startVal;
	}

	// ----------------------------------------------------------

	void TimerBase::Reset()
	{
		m_startVal = GetTicks();
	}

	// ----------------------------------------------------------

	dword TimerBase::Get()
	{
		return (GetTicks() - m_startVal);
	}

	// ----------------------------------------------------------

	void TimerBase::Set(dword interval, bool autoreset)
	{
		m_timeInterval = interval;
		m_autoresetMode = autoreset;

		Reset();
	}

	// ----------------------------------------------------------

	bool TimerBase::TimeOut()
	{
		bool res = (m_timeInterval != 0 && Get() >= m_timeInterval);

		if (res)
		{
			if (m_autoresetMode) Set(m_timeInterval, m_autoresetMode);
			else Stop();
		}

		return res;
	}

	// ----------------------------------------------------------

	void TimerBase::Stop()
	{
		m_autoresetMode = false;
		m_startVal = 0;
		m_timeInterval = 0;  // ?
	}


	// ----------------------------------------------------------

	void TimerBase::Sleep(dword interval)
	{
        Wait(interval);
	}

    // ----------------------------------------------------------

    void TimerBase::Wait(dword interval)
    {
        Set(interval, false);
        while (!TimeOut());
    }


}  // namespace 





