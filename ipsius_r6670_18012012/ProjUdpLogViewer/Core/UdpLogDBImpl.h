#ifndef UDPLOGDBIMPL_H
#define UDPLOGDBIMPL_H

#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "iCore/MsgThread.h"

#include "IUdpPackParser.h"
#include "UdpLogDBParams.h"
#include "UdpLogRecord.h"
#include "UdpLogRecordSrc.h"
#include "UdpLogDBStatus.h"

#include "Utils/ThreadContext.h"

namespace Ulv
{
    // Log DB.
    // Drop part of DB ('dropCount') if DB is full.
    // If IgnoreNewRecordsOnGet() set to true, user will be working with
    // temporary copy of DB.
    // If recording to file enabled, it's recording DB into the file:
    // when part of DB is dropped, when DB is destroyed and on timer ivent
    class UdpLogDBImpl :
        public iCore::MsgObject // for timer
    {
        typedef Utils::ManagedList<UdpLogRecord> List;

        UdpLogDBParams m_params;

        List m_db;
        boost::scoped_ptr<List> m_copyDB;
        // references to records in m_db or in m_copyDB
        std::vector<UdpLogRecord*> m_filteredDB;
        dword m_dropCount;
        ddword m_droppedPacks;
        ddword m_dropTimes;
        bool m_workWithCopy;

        iCore::MsgTimer m_recordingTimer;
        dword m_recordingIndex;
        ddword m_recTimes;

        Utils::ThreadContext m_threadContext;

        int m_debugLevel;

        void OnCreate();
        void OnRecTimer(iCore::MsgTimer *pTimer);

        List& DBToFilter(); // db or copyDB
        int DBToFilterSize() const;
        void DoRefilter(const RecordViewSett &sett);

        void RecordPartOfDB(int size);
        void DropPartOfDB(const RecordViewSett &sett);
        bool IsFull() const;

        void DebugLvl1(const QString &text);

    public:
        UdpLogDBImpl(const UdpLogDBParams &params,
                     iCore::MsgThread &recTimerThread);
        ~UdpLogDBImpl();

        // returns true if packet was successfully parsed
        bool Store(const QString &data,
                   Utils::SafeRef<UdpLogRecordSrc> src,
                   const RecordViewSett &sett,
                   bool &partOfDbWasDropped,
                   UdpLogDBStatus &status);
        void Clear();
        void Refilter(const RecordViewSett &sett);
        void IgnoreNewRecordsOnGet(bool state, const RecordViewSett &sett);

        const UdpLogRecordView& Get(int index,
                                    const RecordViewSett &sett,
                                    int settVersion) const;

        size_t Size() const;        
    };

} // namespace Ulv

#endif // UDPLOGDBIMPL_H
