#ifndef _NOBJ_LOG_MANAGER_H_
#define _NOBJ_LOG_MANAGER_H_


#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

#include "DRI/INonCreatable.h"
#include "iLog/LogManager.h"
#include "iLog/LogWrapper.h"

//#include "NObjLogSettings.h"

namespace Domain
{ 
    // Using to operate LogManager from within session.
	class NObjLogManager : 
        public Domain::NamedObject,
        public DRI::INonCreatable
	{
		Q_OBJECT;
		
		iLogW::LogManager &m_log;

    // DRI setters/getters
    private:
        void TraceToCout(bool traceInd);
        bool TraceToCout() const;

        void TraceToFile(bool traceInd);
        bool TraceToFile() const;

        void FileName(const QString &fileName);
        QString FileName() const;

        void DirectSaveToFile(bool directSaveToFile);
        bool DirectSaveToFile() const;

        void TraceToUdp(bool traceInd);
        bool TraceToUdp() const;

        void DstUdpTrace(const QString &host);
        QString DstUdpTrace() const;

		bool TimestampInd() const;
		void TimestampInd(bool val);

		int CountToSync() const;
		void CountToSync(int countToSync);

		int TimeoutToSync() const;
		void TimeoutToSync(int timeoutToSync);

		int StoreRecordsLimit() const { return m_log.StoreRecordsLimit(); }
		void StoreRecordsLimit(int val) 
		{ 
			if (val > 0 && StoreCleanTo() >= val)
			{
				ThrowRuntimeException("'MaxRecords' and 'CleanTo' conflict.");
			}
			m_log.StoreRecordsLimit(val); 
		}

		int StoreCleanTo() const { return m_log.StoreCleanTo(); }
		void StoreCleanTo(int val) 
		{ 
			int storeLimit = StoreRecordsLimit();
			if (storeLimit > 0 && val >= storeLimit)
			{
				ThrowRuntimeException("'MaxRecords' and 'CleanTo' conflict.");
			}
			m_log.StoreCleanTo(val); 
		}

		QString PathSeparator() const
		{
			return QString::fromStdString(m_log.PathSeparator());
		}
		void PathSeparator(const QString &val)
		{
			if (val.size() <= 0) ThrowRuntimeException("PathSeparator is empty");
			m_log.PathSeparator(val.toStdString());
		}

        QString Stat()
        {
            return m_log.GetStat().ToString("; ").c_str();
        }

	private:        
		iLogW::LogOutputManager &Output()	{	return m_log.Output(); }
		const iLogW::LogOutputManager &Output() const {	return m_log.Output(); }

    // DRI interface
    public:

        Q_PROPERTY(bool TraceToCout READ TraceToCout WRITE TraceToCout);
        Q_PROPERTY(bool TraceToFile READ TraceToFile WRITE TraceToFile);
        Q_PROPERTY(QString FileName READ FileName WRITE FileName);
        Q_PROPERTY(bool DirectSaveToFile READ DirectSaveToFile WRITE DirectSaveToFile);
        Q_PROPERTY(bool TraceToUdp READ TraceToUdp WRITE TraceToUdp);
        Q_PROPERTY(QString DstUdpTrace READ DstUdpTrace WRITE DstUdpTrace);
        Q_PROPERTY(bool Timestamp READ TimestampInd WRITE TimestampInd);
        Q_PROPERTY(int CountToSync READ CountToSync WRITE CountToSync);
        Q_PROPERTY(int TimeoutToSync READ TimeoutToSync WRITE TimeoutToSync);
		Q_PROPERTY(int MaxRecords READ StoreRecordsLimit WRITE StoreRecordsLimit);
		Q_PROPERTY(int CleanTo READ StoreCleanTo WRITE StoreCleanTo);
        Q_PROPERTY(QString PathSeparator READ PathSeparator WRITE PathSeparator);
        Q_PROPERTY(QString Stat READ Stat);

        Q_INVOKABLE void Clear()
        {
            m_log.GetStat(true);
        }

        Q_INVOKABLE void Log(QString s)
        {
			NamedObject::Log() << s << iLogW::EndRecord;
        }

        Q_INVOKABLE void DumpSessionsToFile(QString fileName = "");

	public:

		NObjLogManager(Domain::NamedObject *pParent,
                       const Domain::ObjectName &name);
        
    };
	
	
}  // Domain

#endif
