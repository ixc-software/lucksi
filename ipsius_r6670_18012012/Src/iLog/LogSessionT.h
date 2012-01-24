#ifndef __LOGSESSIONT__
#define __LOGSESSIONT__

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/IBasicInterface.h"
#include "LogRecordT.h"
#include "LogInterfaces.h"
#include "LogSessionsList.h"

namespace iLog
{


    /*
          ��������� �������� ������������������.
          ���������� ��������� ������� ��������, �.�. �������� �������� LogSessionT 
          ������� Assert() � ���������� ������ ��� ���������
    */
    class NullThreadContextCheck
    {
    public:
        void Assert() {}  // nothing
    };

    /* 
        � ������� ���������� ����� ���������� ����� ������ ���������
        ������������� ������ ������ � LogStore, �������� ������������������
        ������ ����� ���������� N �������, ��� �� ���� ������-�� ��������� �������
        ���������� ��� ���������� ������ ����� ������ � ������
    */
    template<class TRecord>
    class ILogSessionSync : public Utils::IBasicInterface
    {
    public:
        virtual bool DoSyncWithStore(TRecord *pRecord) = 0;
    };


    /*
        ������ ������ � �����. 
        �� ������������ ������������������. ������������ ������ ���� � 
        ������������ ������� �� � �������� ���������. ��� ������ ���� ���
        ����� �����. 

        TRecord               -- ��� ������ (������ LogRecordT)
        TThreadContextCheck   -- ����������� ��������� �������� ������ (��. NullThreadContextCheck)
                                 (������ ������ ���� �������� ������ �� ������ ������)

    */
    template<class TRecord, class TThreadContextCheck = NullThreadContextCheck>
    class LogSessionT : 
        boost::noncopyable,
        public ILogSession</*typename*/ TRecord>
    {
        typedef std::list<TRecord*> RecordsList;
        typedef typename TRecord::RecordTimestampMode TimeCaptureModeType;
        typedef typename TRecord::RecordData TData;

        TThreadContextCheck m_contextCheck;
        ISessionToLogStore<TRecord> &m_store;
        LogSessionBody<TData> *m_pBody;
        RecordsList m_list;
        TimeCaptureModeType m_timeCaptureMode;
        RecordKind<TData> m_defaultRecordKind;
        ILogSessionSync<TRecord> *m_pSync;

        bool DoSyncWithStore(TRecord *pRecord)
        {
            if (m_pSync != 0) return m_pSync->DoSyncWithStore(pRecord);
            return true;
        }

        TRecord* CreateRecord(const TData &data, const TData &recordKind)
        {            
            ESS_ASSERT(m_pBody != 0);
            return new TRecord(*m_pBody, data, m_timeCaptureMode, recordKind);
        }

        void DoContextCheck()
        {
            m_contextCheck.Assert();
        }

        void Init(const TData &sessionName,
            const TimeCaptureModeType &timeCaptureMode,
            ILogSessionSync<TRecord> *pSync)
        {
            m_timeCaptureMode = timeCaptureMode;
            m_pBody = m_store.CreateSession(sessionName);

            //TData *pRecKind = m_pBody->RegisterRecordKind( DefaultRecordType() );
            //m_defaultRecordKind = m_pBody->RecordKind(pRecKind, m_pBody);
            m_defaultRecordKind = CreateRecordKind( DefaultRecordType() );

            m_pSync = pSync;
        }

        static TimeCaptureModeType DefaultTimestampMode()
        {
            return TRecord::RecordTimestamp::DefaultCaptureMode();
        }


    // ILogSession<> impl
    public:

        void LogToSession(const TData &data, 
            RecordKind<TData> recordKind = RecordKind<TData>())
        {
            DoContextCheck();

            if (recordKind.Empty()) recordKind = m_defaultRecordKind;
            ESS_ASSERT( recordKind.SessionEqual(m_pBody) );

            TRecord *p = CreateRecord(data, recordKind.Data());
            ESS_ASSERT(p != 0);

            if (DoSyncWithStore(p))
            {
                if (m_list.empty()) m_store.AddRecord(p);
                else
                {
                    m_list.push_back(p);
                    DropSessionToStore();
                }
            }
            else
            {
                m_list.push_back(p);
            }
        }

        void DropSessionToStore()
        {
            DoContextCheck();

            m_store.AddRecords(m_list);
            m_list.clear();
        }

        RecordKind<TData> CreateRecordKind(const TData &data)
        {
            TData *p = m_pBody->RegisterRecordKind(data);

            return RecordKind<TData>(p, m_pBody);
        }

    public:

        LogSessionT(ISessionToLogStore<TRecord> &store, 
            const TData &tagDesc,
            const TimeCaptureModeType &timeCaptureMode = DefaultTimestampMode(),
            ILogSessionSync<TRecord> *pSync = 0) 
            : m_store(store)
        {
            Init(tagDesc, timeCaptureMode, pSync);
        }

        // �������������� �����������, ����������� ������������ char* � �������� �������� ������
        LogSessionT(ISessionToLogStore<TRecord> &store, 
            const char *pTagDesc,
            const TimeCaptureModeType &timeCaptureMode = DefaultTimestampMode(),
            ILogSessionSync<TRecord> *pSync = 0) 
            : m_store(store)
        {
            TData tagDesc;
            tagDesc = TRecord::CharToData(pTagDesc);

            Init(tagDesc, timeCaptureMode, pSync);
        }

        ~LogSessionT()
        {
            DropSessionToStore();

            // LogSession always destroy in unsafe context
            m_pBody->DecUsage(false);
        }

        const TData& getName() const
        {
            ESS_ASSERT(m_pBody != 0);
            return m_pBody->Name();
        }

        /*
        bool TagEqual(Tag tag) const
        {
            return (m_tag == tag);
        } */

        static typename TRecord::RecordData DefaultRecordType()
        {
            return TRecord::CharToData("<default>");
        }

        LogSessionBody<TData>* SessionBody() const { return m_pBody; }

    };

}  // namespace iLog

#endif

