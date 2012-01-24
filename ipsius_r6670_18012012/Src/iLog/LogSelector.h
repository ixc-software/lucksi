#ifndef __LOGSELECTOR__
#define __LOGSELECTOR__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"

namespace iLog
{

    // фильтр по временной метке
    template<class TRecord>
    class ITimestampFilter : public Utils::IBasicInterface
    {
    public:
        virtual bool TimestampAccepted(const typename TRecord::RecordTimestamp &ts) = 0;
    };

    // фильтр по описанию тэга
    template<class TData>
    class ISessionFilter : public Utils::IBasicInterface
    {
    public:
        virtual bool SessionAccepted(const TData &sessionName) = 0;
    };

    // фильтр по всей записи
    template<class TRecord>
    class IRecordFilter : public Utils::IBasicInterface
    {
    public:
        virtual bool RecordAccepted(TRecord *pRecord) = 0;
    };

    // ---------------------------------------

    /*
        Интефрейс для описания выборки записей из LogStore
    */
    template<class TRecord>
    class ILogSelector : 
        public ISessionFilter<typename TRecord::RecordData>, 
        public IRecordFilter<TRecord>
    {
    };

    // ---------------------------------------


    /* 
        Класс, обеспечивающий критерии выборки из LogStore

        TRecord               -- тип записи (шаблон LogRecordT)
    */
    template<class TRecord>
    class LogSelector : 
        boost::noncopyable, 
        public ILogSelector<TRecord>
    {
        typedef typename TRecord::RecordData TData;

        ITimestampFilter<TRecord>   *m_pFilterTS;
        ISessionFilter<TData>       *m_pFilterSession;
        IRecordFilter<TRecord>      *m_pFilterRecord;

        void Init()
        {
            m_pFilterTS = 0; 
            m_pFilterSession = 0; 
            m_pFilterRecord = 0;
        }


    // ILogSelector<TRecord> impl
    private:

        bool SessionAccepted(const TData &sessionName)
        {
            if (m_pFilterSession != 0) return m_pFilterSession->SessionAccepted(sessionName);
            return true;
        }

        bool RecordAccepted(TRecord *pRecord)
        {
            if (m_pFilterTS != 0)
            {
                if (!m_pFilterTS->TimestampAccepted( pRecord->Timestamp() )) return false;
            }

            if (m_pFilterRecord != 0)
            {
                if (!m_pFilterRecord->RecordAccepted(pRecord)) return false;
            }

            return true;
        }

    public:
        LogSelector()
        {
            Init();
        }

        LogSelector(ITimestampFilter<TRecord> &tsFilter)
        {
            Init();
            m_pFilterTS = &tsFilter;
        }

        LogSelector(ISessionFilter<TRecord> &sessionFilter)
        {
            Init();
            m_pFilterSession = &sessionFilter;
        }

        LogSelector(ITimestampFilter<TRecord> &tsFilter, ISessionFilter<TData> &sessionFilter)
        {
            Init();
            m_pFilterTS = &tsFilter;
            m_pFilterSession = &sessionFilter;
        }

    };

    // ---------------------------------------

    // фильтр min, max, [min .. max] для временной метки
    template<class TRecord>
    class LogTimestampFilter : public ITimestampFilter<TRecord>
    {
    public:

        typedef typename TRecord::RecordTimestamp Timestamp;
        typedef typename TRecord::RecordData TagDesc;

        LogTimestampFilter(const Timestamp &ts, bool asMin)
        {
            Timestamp emptyTs = EmptyTimestamp();

            if (asMin) Init(ts, emptyTs);
                  else Init(emptyTs, ts);

        }

        LogTimestampFilter(const Timestamp &minTimestamp, 
                           const Timestamp &maxTimestamp)
        {
            Init(minTimestamp, maxTimestamp);
        }


    // ITimestampFilter<> impl
    private:

        bool TimestampAccepted(const typename TRecord::RecordTimestamp &ts)
        {
            if (!m_min.IsEmpty())
            {
                if (ts.Compare(m_min) < 0) return false;
            }

            if (!m_max.IsEmpty())
            {
                if (ts.Compare(m_max) > 0) return false;
            }

            return true;
        }

    private:

        Timestamp m_min;
        Timestamp m_max;

        static Timestamp EmptyTimestamp()
        {
            return TRecord::RecordTimestamp::EmptyTimestamp();
        }

        void Init(const Timestamp &minTimestamp, 
                  const Timestamp &maxTimestamp)
        {
            m_min = minTimestamp;
            m_max = maxTimestamp;
        }

    };



}  // namespace iLog

#endif

