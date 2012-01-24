#ifndef __LOGSTORET__
#define __LOGSTORET__

#include "stdafx.h"
#include "Utils/ManagedList.h"
#include "Utils/IVirtualDestroy.h"
#include "LogRecordT.h"
#include "LogInterfaces.h"
#include "LogSelector.h"
#include "LogSessionsList.h"
#include "LogSessionT.h"
#include "LogStoreStrategy.h"
#include "LogStat.h"

namespace iLog
{

    // макет стратегии обеспечени€ потокобезопасности
    class ThreadUnsafeLockStrategy
    {
    public:

        class Mutex {};

        // RAII locker for mutex 
        class Locker
        {
        public:
            Locker(Mutex &m) {}
        };

    };


    /*
        √лавный класс -- лог, хранилище записей

        TRecord             -- тип записи (шаблон LogRecordT)

        TLockStrategy       -- стратеги€ реализации многопоточности (см. ThreadUnsafeLockStrategy)
    */
    template
        <class TRecord, 
        class TLockStrategy = ThreadUnsafeLockStrategy>
    class LogStoreT : 
        boost::noncopyable, 
        public ISessionToLogStore<TRecord>,
        ILogStoreAccess<TRecord>,
        ISessionBodyDestroy
    {
        typedef typename TRecord::RecordData    TData;
        typedef typename TRecord::RecordTag     TRecordTag;
        typedef typename TLockStrategy::Locker  QLocker;
        typedef typename TLockStrategy::Mutex   TMutex;
        typedef typename LogSessionsList<TRecord>::LogSessionsSet LogSessionsSet;
        typedef typename TRecord::RecordTimestamp::CaptureModeType TsCaptureMode;

        typedef std::list<TRecord*> RecordsList;

        TMutex m_mutex;

        LogSessionsList<TRecord> m_sessions;  // must be upper m_list - !
        RecordsList m_list;        
        LogSessionT<TRecord> m_systemSession;
        ILogStoreStrategy<TRecord> *m_pStrategy;
        LogStat m_stat;

        // options
        const bool m_useRecordsReorder;
        const bool m_dublicateSessionNamesDetection;

        /*
            јлгоримт поиска места дл€ вставки pRecord

            (начало) [e][e][e][1][e][e][X][3][e][e] (конец)   <-- направление просмотра
            [e] -- элементы с пустым timestamp
            [X] -- место вставки элемента со значением 2, 
                   итератор-результат при этом указывает на [3]
        */
        typename RecordsList::iterator SearchInsertPlaceByTimestamp(TRecord *pRecord)
        {
            typename RecordsList::iterator i = m_list.end();
            typename RecordsList::iterator lastBigger = m_list.end();

            while(i != m_list.begin())
            {
                --i;
                TRecord *pCurr = (*i);
                if (pCurr->TimeStampEmpty()) continue;

                int cmp = pRecord->TimeStampCompare(*pCurr);
                if (cmp < 0) lastBigger = i;
                        else break;                    

            }

            return lastBigger;
        }

        /*
             Ќовые записи с пустым timestamp всегда вставл€ютс€ в конец списка,
             вставка элементов со временем определ€етс€ алгоритмом SearchInsertPlaceByTimestamp()
        */
        void SafeAddRecord(TRecord *pRecord)
        {
            ESS_ASSERT(pRecord != 0);

            m_stat.TotalRecords++;
            m_stat.TotalRecordsData += pRecord->Data().size();

            if (m_pStrategy != 0)
            {
                bool add = m_pStrategy->OnNewRecord(*this, *pRecord);
                if (!add)
                {
                    delete pRecord;
                    return;
                }
            }

            m_stat.TotalRecordsStored++;

            if (pRecord->TimeStampEmpty() || (!m_useRecordsReorder))
            {
                m_list.push_back(pRecord);
            }
            else
            {
                typename RecordsList::iterator i = SearchInsertPlaceByTimestamp(pRecord);
                m_list.insert(i, pRecord);
            }
           
        }

        void OnChange()
        {
            // nothing...
        }

        void SystemLog(const TData &s)
        {
            m_systemSession.LogToSession(s);
        }

        static TsCaptureMode SystemCaptureMode()
        {
            return TRecord::RecordTimestamp::DefaultSystemCaptureMode();
        }

        void WarningDublicateTagDescription(const TData &desc)
        {
            TData s = TRecord::CharToData("Dublicate desciption registred, ");
            s += desc;
            SystemLog(s);
        }

    // ISessionToLogStore<> impl
    public:

        void AddRecord(TRecord *pRecord)
        {
            // recursive mutex lock workaround, we can safely add records for system session
            if (m_systemSession.SessionBody() == pRecord->SessionBody())
            {
                SafeAddRecord(pRecord);
                return;
            }

            // add thru lock
            {
                QLocker locker(m_mutex);
                SafeAddRecord(pRecord);
            }

            OnChange();
        }

        void AddRecords(std::list<TRecord*> &list)
        {
            {
                QLocker locker(m_mutex);

                for(typename RecordsList::iterator i = list.begin(); i != list.end(); ++i)
                {
                    SafeAddRecord(*i);
                }
            }

            OnChange();
        }

        LogSessionBody<TData>* CreateSession(const TData &sessionName)
        {
            QLocker locker(m_mutex);

            m_stat.TotalSessions++;

            if (m_dublicateSessionNamesDetection)
            {
                if (m_sessions.Exists(sessionName))
                {
                    WarningDublicateTagDescription(sessionName);
                }
            }

            return m_sessions.Register(sessionName); //   m_tags.RegisterNew(tagDesc);
        }

    // ILogStoreAccess<> impl
    // Where is no mutex lock -- this interface avaible only in safe context
    private:

        std::list<TRecord*>& StoredRecords()
        {
            return m_list;
        }

        void StoreOnlyLast(int records)
        {
            int recordsToDelete = m_list.size() - records;
            if (recordsToDelete <= 0) return;

            // free items
            typename RecordsList::iterator i = m_list.begin();

            while(recordsToDelete--)
            {
                TRecord *p = (*i);
                delete p;

                ++i;
            }

            // remove items from list
            m_list.erase(m_list.begin(), i);
        }


    // ISessionBodyDestroy impl
    private:

        void SafeSessionDestroy(Utils::IVirtualDestroy *pObject)
        {
            ESS_ASSERT(pObject != 0);
            m_sessions.Unregister(pObject);
        }

        void UnsafeSessionDestroy(Utils::IVirtualDestroy *pObject)
        {
            QLocker locker(m_mutex);

            SafeSessionDestroy(pObject);
        }

    public:

        LogStoreT(ILogStoreStrategy<TRecord> *pStrategy = 0, 
            bool useRecordsReorder = true,
            bool dublicateSessionNamesDetection = true) : 
            m_sessions(*this),
            m_systemSession(*this, SystemTagDesc(), SystemCaptureMode()),
            m_pStrategy(pStrategy), 
            m_useRecordsReorder(useRecordsReorder),
            m_dublicateSessionNamesDetection(dublicateSessionNamesDetection)
        {
            // if (m_pStrategy != 0) m_pStrategy->SetStoreAccess(*this);
        }

        ~LogStoreT()
        {
            if (m_pStrategy != 0) m_pStrategy->OnFinalize(*this);

            // free records
            StoreOnlyLast(0);

            /*
            for(typename RecordsList::iterator i = m_list.begin(); i != m_list.end(); ++i)
            {
                TRecord *p = (*i);
                delete p;
            }
            m_list.clear(); */
        }

        int Select(ILogSelector<TRecord> &selector, std::list<TRecord*> &result)
        {
            QLocker locker(m_mutex);

            // select sessions
            LogSessionsSet selectedSessions;
            if (m_sessions.MakeSelectionSet(selector, selectedSessions) == 0) return 0;

            // filter records
            int count = 0;
            for(typename RecordsList::iterator i = m_list.begin(); i != m_list.end(); ++i)
            {
                TRecord *p = (*i);

                {
                    typename LogSessionsSet::iterator i = 
                        std::find(selectedSessions.begin(), selectedSessions.end(), p->SessionBody());
                    if (i == selectedSessions.end()) continue;
                }
                
                if (selector.RecordAccepted(p)) 
                {
                    result.push_back(p);
                    count++;
                }
            }

            return count;
        }

        int getCount()
        {
            QLocker locker(m_mutex);

            return m_list.size();
        }

        LogStat GetStat(bool withClear = false) 
        {
            QLocker locker(m_mutex);

            if (withClear) StoreOnlyLast(0);

            LogStat s = m_stat;
            s.CurrRecords = m_list.size();
            s.CurrSessions = m_sessions.Size();

            return s;
        }

        // debug
        void DumpSessionNames(std::vector<std::string> &dump)
        {
            QLocker locker(m_mutex);
            m_sessions.DumpNames(dump);
        }


        static TData SystemTagDesc()
        {
            return TRecord::CharToData("LogStoreSystem");
        }

    };
	
}  // namespace iLog


#endif

