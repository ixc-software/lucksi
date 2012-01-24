
#include "stdafx.h"

#include "UdpLogDBImpl.h"


namespace Ulv
{
    UdpLogDBImpl::UdpLogDBImpl(const UdpLogDBParams &params,                               
                               iCore::MsgThread &recTimerThread) :
        iCore::MsgObject(recTimerThread),
        m_params(params),
        m_db(true, params.RecordCount()),
        m_dropCount(params.DropRecordCount()),
        m_droppedPacks(0),
        m_dropTimes(0),
        m_workWithCopy(false),
        m_recordingTimer(this, &UdpLogDBImpl::OnRecTimer),
        m_recordingIndex(0),
        m_recTimes(0),
        m_threadContext(false)
    {
        m_debugLevel = 0;

        m_filteredDB.reserve(params.RecordCount());

        // created and working in diff threads
        PutMsg(this, &UdpLogDBImpl::OnCreate);
    }

    // -----------------------------------------------------------------

    UdpLogDBImpl::~UdpLogDBImpl()
    {        
        m_filteredDB.clear();
        m_copyDB.reset();
        m_db.Clear();
    }

    // -----------------------------------------------------------------

    void UdpLogDBImpl::OnCreate()
    {
        m_threadContext.Set();

        if (m_params.RecordingEnabled())
        {
            m_recordingTimer.Start(m_params.RecordingIntervalMs(), true);
        }
    }

    // -----------------------------------------------------------------

    UdpLogDBImpl::List& UdpLogDBImpl::DBToFilter()
    {
        if (!m_workWithCopy)
        {
            // DebugLvl1("DBToFilter -> main");
            return m_db;
        }

        ESS_ASSERT(m_copyDB != 0);
        // DebugLvl1("DBToFilter -> copy");
        return *m_copyDB.get();
    }

    // -----------------------------------------------------------------

    int UdpLogDBImpl::DBToFilterSize() const
    {
        if (!m_workWithCopy) return m_db.Size();

        if (m_copyDB == 0) return 0;

        return m_copyDB->Size();
    }

    // -----------------------------------------------------------------

    void UdpLogDBImpl::OnRecTimer(iCore::MsgTimer *pTimer)
    {
        // m_threadContext.Set();

        RecordPartOfDB(m_db.Size());
    }

    // -----------------------------------------------------------------

    void UdpLogDBImpl::RecordPartOfDB(int size)
    {
        DebugLvl1("RecordPartOfDB");

        ++m_recTimes;

        m_threadContext.Assert();

        if (!m_params.RecordingEnabled() || m_db.Size() == 0) return;


        RecordViewSett sett = m_params.Recorder().Format();
        int settVer = -1;

        int end = m_recordingIndex + size;
        if (end > m_db.Size()) end = m_db.Size();

        QString res;
        const int CTypicalRecSize = 256;
        res.reserve(size * CTypicalRecSize);
        for (int i = m_recordingIndex; i < end; ++i)
        {
            res += m_db[i]->View(sett, settVer).Get();
            res += "\n";
        }

        m_params.Recorder().Add(res);
        m_recordingIndex = end;

        DebugLvl1("\\RecordPartOfDB");
    }

    // -----------------------------------------------------------------

    void UdpLogDBImpl::DoRefilter(const RecordViewSett &sett)
    {
        DebugLvl1("DoRefilter");
        
        m_filteredDB.clear();

        int size = DBToFilterSize();

        for (size_t i = 0; i < size; ++i)
        {
            UdpLogRecord *pCurr = DBToFilter()[i];
            ESS_ASSERT(pCurr != 0);

            if (pCurr->Filter(sett)) m_filteredDB.push_back(pCurr);            
        }
        
        DebugLvl1("\\DoRefilter");
    }

    // -----------------------------------------------------------------

    void UdpLogDBImpl::DropPartOfDB(const RecordViewSett &sett)
    {
        DebugLvl1("DropPartOfDB");

        ESS_ASSERT(IsFull());

        if (!m_workWithCopy) m_filteredDB.clear();

        {
            ++m_dropTimes;

            size_t dbFullCapacity = m_db.Capacity();

            RecordPartOfDB(m_db.Size());

            m_db.Delete(0, m_dropCount);
            m_droppedPacks += m_dropCount;

            m_recordingIndex = m_dropCount;

            // expand to full capacity
            m_db.Reserve(dbFullCapacity);
        }

        if (!m_workWithCopy) DoRefilter(sett);

         DebugLvl1("\\DropPartOfDB");
    }

    // -----------------------------------------------------------------

    bool UdpLogDBImpl::IsFull() const
    {
        return (m_db.Size() == m_db.Capacity());
    }

    // -----------------------------------------------------------------

    void UdpLogDBImpl::DebugLvl1(const QString &text)
    {
        if (m_debugLevel != 1) return;

        QString msg = QString("Debug [UdpLogDBImpl]: %1").arg(text);
        std::cout << msg.toStdString() << std::endl;
    }

    // -----------------------------------------------------------------

    bool UdpLogDBImpl::Store(const QString &data,
                             Utils::SafeRef<UdpLogRecordSrc> src,
                             const RecordViewSett &sett,
                             bool &partOfDbWasDropped,
                             UdpLogDBStatus &status)
    {
        m_threadContext.Assert();

        partOfDbWasDropped = false;

        if (((m_db.Size() - m_recordingIndex)  == m_params.RecordingRecordCount())
            && (m_params.RecordingEnabled()))
        {
            RecordPartOfDB(m_params.RecordingRecordCount());
            status.Recording(m_recTimes, m_params.Recorder().RecordedSize());
        }

        if (IsFull())
        {
            DropPartOfDB(sett);
            if (!m_workWithCopy) partOfDbWasDropped = true;
            if (m_params.RecordingEnabled())
            {
                status.Recording(m_recTimes, m_params.Recorder().RecordedSize());
            }
        }

        // will be deleted in m_db destructor
        UdpLogRecord *pRec = new UdpLogRecord(data, src, m_params.Parser());

        // add
        m_db.Add(pRec);

        // add to filter
        if ((!m_workWithCopy) && pRec->Filter(sett)) m_filteredDB.push_back(pRec);

        // update status
        status.ReceiveNewPack(data.size(), pRec->IsSuccessfullyParsed());
        status.FilteredCount(Size());
        status.PacksInDBCount(m_db.Size());
        status.DroppedCount(m_droppedPacks, m_dropTimes);

        return pRec->IsSuccessfullyParsed();
    }

    // -----------------------------------------------------------------

    void UdpLogDBImpl::Clear()
    {
        DebugLvl1("Clear");

        m_threadContext.Assert();

        RecordPartOfDB(m_db.Size());

        size_t dbCapacity = m_db.Capacity();

        m_filteredDB.clear();
        m_copyDB.reset();
        m_db.Clear();

        m_db.Reserve(dbCapacity);
        m_filteredDB.reserve(dbCapacity);

        m_recordingIndex = 0;

        DebugLvl1("\\Clear");
    }

    // -----------------------------------------------------------------

    void UdpLogDBImpl::Refilter(const RecordViewSett &sett)
    {
        DebugLvl1("Refilter");

        DoRefilter(sett);

        DebugLvl1("\\Refilter");
    }

    // -----------------------------------------------------------------

    void UdpLogDBImpl::IgnoreNewRecordsOnGet(bool state, const RecordViewSett &sett)
    {
        DebugLvl1("IgnoreNewRecordsOnGet");

        m_threadContext.Assert();

        m_workWithCopy = state;
        m_filteredDB.clear();
        m_copyDB.reset();

        if (m_workWithCopy)
        {
            m_copyDB.reset(new List(true));
            m_copyDB->Reserve(m_db.Capacity());
            for (int i = 0; i < m_db.Size(); ++i)
            {                
                m_copyDB->Add(new UdpLogRecord(*m_db[i]));
            }
        }

        DoRefilter(sett);

        DebugLvl1("\\IgnoreNewRecordsOnGet");
    }

    // -----------------------------------------------------------------

    const UdpLogRecordView& UdpLogDBImpl::Get(int index,
                                              const RecordViewSett &sett,
                                              int settVersion) const
    {
        m_threadContext.Assert();

        ESS_ASSERT((index >= 0) && (index < m_filteredDB.size()));
        UdpLogRecord *pCurr = m_filteredDB.at(index);
        ESS_ASSERT(pCurr != 0);

        return pCurr->View(sett, settVersion);
    }

    // -----------------------------------------------------------------

    size_t UdpLogDBImpl::Size() const
    {
        return m_filteredDB.size();
    }

    // -----------------------------------------------------------------

} // namespace Ulv
