#ifndef _NOBJ_LOG_SETTINGS_H_
#define _NOBJ_LOG_SETTINGS_H_

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

#include "DRI/INonCreatable.h"
#include "iLog/LogManager.h"
#include "iLog/LogSettings.h"

namespace Domain
{ 
    // Using to operate LogManager from within session.
	class NObjLogSettings : 
        public Domain::NamedObject,
        public DRI::INonCreatable
	{
		Q_OBJECT;
		
		iLogW::LogSettings m_settings;
    // DRI setters/getters
    public:
		bool TimestampInd() const
		{
			return m_settings.TimestampInd();
		}
		void TimestampInd(bool val)
		{
			m_settings.TimestampInd(val);
		}

		int CountToSync() const
		{
			return m_settings.CountSyncroToStore();
		}

		void CountToSync(int countToSync)
		{
			m_settings.CountSyncroToStore(countToSync);
		}
		int TimeoutToSync() const
		{
			return m_settings.TimeoutSyncroToStore();
		}
		void TimeoutToSync(int timeoutToSync)
		{
			m_settings.TimeoutSyncroToStore(timeoutToSync);
		}
		int StoreRecordsLimit() const { return m_settings.StoreRecordsLimit(); }
		void StoreRecordsLimit(int val) 
		{ 
			if (val > 0 && StoreCleanTo() >= val)
			{
				ThrowRuntimeException("'MaxRecords' and 'CleanTo' conflict.");
			}
			m_settings.StoreRecordsLimit(val); 
		}

		int StoreCleanTo() const { return m_settings.StoreCleanTo(); }
		void StoreCleanTo(int val) 
		{ 
			int storeLimit = StoreRecordsLimit();
			if (storeLimit > 0 && val >= storeLimit)
			{
				ThrowRuntimeException("'MaxRecords' and 'CleanTo' conflict.");
			}
			m_settings.StoreCleanTo(val); 
		}

		QString PathSeparator() const
		{
			return QString::fromStdString(m_settings.PathSeparator());
		}
		void PathSeparator(const QString &val)
		{
			if (val.size() <= 0) ThrowRuntimeException("PathSeparator is empty");
			m_settings.PathSeparator(val.toStdString());
		}
    // DRI interface
    public:
        Q_PROPERTY(bool Timestamp READ TimestampInd WRITE TimestampInd);
        Q_PROPERTY(int CountToSync READ CountToSync WRITE CountToSync);
        Q_PROPERTY(int TimeoutToSync READ TimeoutToSync WRITE TimeoutToSync);
		Q_PROPERTY(int MaxRecords READ StoreRecordsLimit WRITE StoreRecordsLimit);
		Q_PROPERTY(int CleanTo READ StoreCleanTo WRITE StoreCleanTo);
        Q_PROPERTY(QString PathSeparator READ PathSeparator WRITE PathSeparator);
	public:
		NObjLogSettings(Domain::NamedObject *pParent,
			const Domain::ObjectName &name) : 
			Domain::NamedObject(&pParent->getDomain(), name, pParent)
		{}
		iLogW::LogSettings LogSettings() 
		{
			m_settings.out() = getDomain().Log().Output().Settings();
			return m_settings;
		}
    };
	
	
}  // Domain

#endif
