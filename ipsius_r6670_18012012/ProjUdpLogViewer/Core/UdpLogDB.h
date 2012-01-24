
#ifndef __LOGVIEW_UDPLOGDB__
#define __LOGVIEW_UDPLOGDB__


#include "UdpLogDBImpl.h"
#include "UdpLogDBStatus.h"

namespace Ulv
{
    // DataBase of UdpPackets. Has fixed size.
    // until it size less then DB capacity.
    // All data will be deleted from buffer after it was full.
    // Percent of DB will be droped after it has no more space to store new packets.
    class UdpLogDB : 
        boost::noncopyable
    {
        RecordViewSett m_recViewSett;
        int m_settVersion;

        UdpLogRecordSrcList m_srcDB;
        UdpLogDBImpl m_db;

        UdpLogDBStatus m_status;

        int m_debugLevel;

        void ToString(int start, int count, QString &res) const;
        void ForceRefilter();

        void DebugLvl1(const QString &text) const;

    public:
        UdpLogDB(const UdpLogDBParams &params, iCore::MsgThread &recTimerThread);

        // return true if packet was successfully parsed
        bool Store(const QString &data, const Utils::HostInf &src, bool &partOfDbWasDropped);
        void IgnoreNewRecordsOnGet(bool state);
        void Clear();

        // return true if DB was refiltered
        bool UpdateSettings(const RecordViewSett &sett);

        // return true if DB was refiltered
        bool SyncRecordSrcSelection(const UdpLogRecordSrcInfoList &list);

        void UpdateStatus(ddword refreshMsgCount,
                          ddword renderImgCount, ddword renderImgAvgTimeMs);

        const UdpLogRecordView& RecordView(int index) const;
        size_t RecordCount() const;

        //const Ulv::UdpLogRecordSrc& RecordSrc(size_t index) const;
        //size_t SrcCount() const;

        const UdpLogRecordSrcList& RecordSrcList() const { return m_srcDB; }

        const UdpLogDBStatus& Status() const { return m_status; }

        // void CopyToClipboard(QClipboard &cb, size_t start, size_t count);
        // void SaveToFile(const QString &fileName, size_t start, size_t count, QString &err);
        QString GetCopy(int start = 0, int count = -1);
    };    

} // namespace Ulv

#endif
