
#ifndef __NOBJTHREADMONITOR__
#define __NOBJTHREADMONITOR__

// NObjThreadMonitor.h

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "iCore/MsgThreadMonitor.h"
#include "DRI/INonCreatable.h"

namespace Domain
{
    // Using to operate MsgThreadMonitor from within session.
    class NObjThreadMonitor :
        public Domain::NamedObject,
        public DRI::INonCreatable
	{
		Q_OBJECT;

        iCore::MsgThreadMonitor m_monitor;

    // DRI getters
    private:
        int MaxMsgQueueSize() const { return m_monitor.MaxMsgQueueSize(); }
        int MaxTimerLagCount() const { return m_monitor.MaxTimerLagCount(); }
        int MaxTimerLag() const { return m_monitor.MaxTimerLag(); }
        int MsgGrowCount() const { return m_monitor.MsgGrowCount(); }
        int MsgGrowLimit() const { return m_monitor.MsgGrowLimit(); }

        QString Statistic() const 
        { 
            return QString("[%1]").arg(QString::fromStdString(m_monitor.getInfo())); 
        }
        
    // DRI interface
    public:
        Q_INVOKABLE void SetMaxMsgQueueSize(int msgCount) 
        { 
            m_monitor.SetMaxMsgQueueSize(msgCount);
        }

        Q_INVOKABLE void ResetMaxMsgQueueSize() { m_monitor.ResetMaxMsgQueueSize(); }

        Q_INVOKABLE void SetMaxTimerLag(int timerLagCount, int maxTimerLag)
        {
            m_monitor.SetMaxTimerLag(timerLagCount, maxTimerLag);
        }

        Q_INVOKABLE void ResetMaxTimerLag() { m_monitor.ResetMaxTimerLag(); }

        Q_INVOKABLE void SetMsgGrowDetection(int msgGrowCount, int msgGrowLimit)
        {
            m_monitor.SetMsgGrowDetection(msgGrowCount, msgGrowLimit);
        }
        
        Q_INVOKABLE void ResetMsgGrowDetection() { m_monitor.ResetMsgGrowDetection(); }

        Q_INVOKABLE void Reset() { m_monitor.Clear(); }
        
		Q_PROPERTY(int MaxMsgQueueSize READ MaxMsgQueueSize);
        Q_PROPERTY(int MaxTimerLagCount READ MaxTimerLagCount);
        Q_PROPERTY(int MaxTimerLag READ MaxTimerLag);
        Q_PROPERTY(int MsgGrowCount READ MsgGrowCount);
        Q_PROPERTY(int MsgGrowLimit READ MsgGrowLimit);
        Q_PROPERTY(QString Statistic READ Statistic);

    public:
		NObjThreadMonitor(Domain::NamedObject *pParent,
                          const Domain::ObjectName &name) :
            NamedObject(&pParent->getDomain(), name, pParent),
            m_monitor(pParent->getDomain().getMsgThread())
        {
        }

        iCore::MsgThreadMonitor& Monitor() { return m_monitor; }
    };
    
} // namespace Domain

#endif
