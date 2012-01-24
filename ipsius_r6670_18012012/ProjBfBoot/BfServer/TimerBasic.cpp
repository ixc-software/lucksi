#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "TimerBasic.h"


using namespace Platform;


namespace BfServer
{
    TimerBasic::TimerBasic(IExecuter &owner, ITimerEvents &observer) :
        ExecutionUnit(owner),
        m_observer(observer),
        m_enabled(false),
        m_repeated(false),
        m_startedAt(0),
        m_interval(0)
    {
    }


    //-------------------------------------------------------------------------


    TimerBasic::~TimerBasic()
    {
    }


    //-------------------------------------------------------------------------


    void TimerBasic::Start(dword interval, bool repeated)
    {
        ESS_ASSERT(interval > 0);

        m_startedAt = GetSystemTickCount();
        m_interval = interval;

        m_repeated = repeated;
        m_enabled = true;
    }


    //-------------------------------------------------------------------------


    void TimerBasic::Stop()
    {
        m_enabled = false;
    }


    //-------------------------------------------------------------------------


    bool TimerBasic::Enabled() const
    {
        return m_enabled;
    }


    //-------------------------------------------------------------------------
    // ExecutionUnit implementation

    void TimerBasic::OnExecute()
    {
        if ( m_enabled && (GetSystemTickCount() - m_startedAt >= m_interval) )
        {
            m_observer.OnTimer(*this);

            if (m_repeated)
                m_startedAt = GetSystemTickCount();
            else
                Stop();
        }
    }


} // namespace BfServer
