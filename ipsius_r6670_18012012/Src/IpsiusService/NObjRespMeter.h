#pragma once

#include "iCore/MsgTimer.h"
#include "Utils/StatisticElement.h"
#include "Domain/NamedObject.h"

namespace IpsiusService
{

    class NObjRespMeter : public Domain::NamedObject
    {
        Q_OBJECT;

        iCore::MsgTimer m_timer;

        int m_interval;
        int m_criticalLevel;

        int m_startTime;
        Utils::StatElementForInt m_process;
        int m_criticalCount;

        static int Ticks() { return Platform::GetSystemTickCount(); }

        void OnTimer(iCore::MsgTimer *pT)
        {
            int elapsed = Ticks() - m_startTime;

            int counter = 0;
            int sum = m_process.Empty() ? 0 : m_process.Sum();

            while ( m_interval * (sum + counter) < elapsed )
            {
                ++counter;
            }

            m_process.Add(counter);
            if (counter >= m_criticalLevel) ++m_criticalCount;
        }

    public:

        NObjRespMeter(Domain::IDomain *pDomain, const Domain::ObjectName &name) : 
          Domain::NamedObject(pDomain, name),
          m_timer(this, &NObjRespMeter::OnTimer),
          m_interval(0),
          m_criticalLevel(0)
        {
            Reset();
        }

        Q_INVOKABLE void Start(int interval = 25, int criticalLevel = 3)
        {
            Reset();

            m_interval = interval;
            m_criticalLevel = criticalLevel;

            m_timer.Start(m_interval, true);
        }

        Q_INVOKABLE void Stop()
        {
            m_timer.Stop();

            m_interval = 0;
        }

        Q_INVOKABLE void Reset()
        {
            m_startTime = Ticks();
            m_process.Clear();
            m_criticalCount = 0;
        }

        Q_INVOKABLE void Info(DRI::ICmdOutput *pCmd) const
        {
            if (m_interval == 0)
            {
                pCmd->Add("Not active!");
                return;
            }

            float avg = (float)m_process.Sum() / m_process.Count();
            QString s = QString("Elapsed %1 seconds, avg %2, critial %3 times (interval %4, critical level %5)")
                .arg((Ticks() - m_startTime) / 1000)
                .arg(avg, 0, 'g', 2)
                .arg(m_criticalCount)
                .arg(m_interval)
                .arg(m_criticalLevel);

            pCmd->Add(s);
        }

    };
    
    
}  // namespace IpsiusService
