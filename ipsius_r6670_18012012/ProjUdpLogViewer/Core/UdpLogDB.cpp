
#include "stdafx.h"
#include "UdpLogDB.h"
#include "UlvUtils.h"


namespace Ulv
{
    UdpLogDB::UdpLogDB(const UdpLogDBParams &params, iCore::MsgThread &recTimerThread) :
        m_settVersion(0),
        m_db(params, recTimerThread)
    {
        m_debugLevel = 0;
    }

    // --------------------------------------------------------------------------------

    bool UdpLogDB::Store(const QString &data, const Utils::HostInf &src, bool &partOfDbWasDropped)
    {
        // src
        Utils::SafeRef<UdpLogRecordSrc> recSrc = m_srcDB.AddOrUpdate(src, data.size(), true);

        // record
        bool okParsed = m_db.Store(data, recSrc, m_recViewSett, partOfDbWasDropped, m_status);

        return okParsed;
    }

    // --------------------------------------------------------------------------------

    void UdpLogDB::IgnoreNewRecordsOnGet(bool state)
    {
        m_db.IgnoreNewRecordsOnGet(state, m_recViewSett);
    }

    // --------------------------------------------------------------------------------

    bool UdpLogDB::UpdateSettings(const RecordViewSett &sett)
    {
        DebugLvl1("UpdateSettings");

        FilterExp oldFilter = m_recViewSett.Filter();

        if (m_recViewSett != sett) ++m_settVersion;
        m_recViewSett = sett;

        if (oldFilter == sett.Filter())
        {
            DebugLvl1("\\UpdateSettings");
            return false;
        }
        
        ForceRefilter();

        DebugLvl1("\\UpdateSettings");        
        return true;
    }

    // --------------------------------------------------------------------------------

    bool UdpLogDB::SyncRecordSrcSelection(const UdpLogRecordSrcInfoList &list)
    {
        DebugLvl1("SyncRecordSrcSelection");

        bool wasChanged = m_srcDB.SyncSelection(list);

        if (wasChanged) ForceRefilter();

        DebugLvl1("\\SyncRecordSrcSelection");

        return wasChanged;
    }

    // --------------------------------------------------------------------------------

    void UdpLogDB::Clear()
    {
        m_db.Clear();
        m_srcDB.Clear();
        m_status.Clear();
    }

    // --------------------------------------------------------------------------------

    const UdpLogRecordView& UdpLogDB::RecordView(int index) const
    {
        return m_db.Get(index, m_recViewSett, m_settVersion);
    }

    // --------------------------------------------------------------------------------

    size_t UdpLogDB::RecordCount() const
    {
        return m_db.Size();
    }

    // --------------------------------------------------------------------------------

    /*void UdpLogDB::CopyToClipboard(QClipboard &cb, size_t start, size_t count)
    {
        DebugLvl1("CopyToClipboard");

        QString res;

        // fix: 'start' is sent from other thead and DB can be dropped before we received it
        if (start > RecordCount()) start = 0;
        if (count > (RecordCount() - start)) count = RecordCount();

        // count = 0 -> use dbSize as count
        if (count == 0) count = RecordCount();


        ToString(start, count, res);
        // cb.setText(res);

        DebugLvl1("\\CopyToClipboard");
    }

    // --------------------------------------------------------------------------------

    void UdpLogDB::SaveToFile(const QString &fileName, size_t start, size_t count,
                              QString &err)
    {
        DebugLvl1("SaveToFile");

        QString res;
        ToString(start, count, res);

        UlvUtils::SaveToFile(fileName, res, false, err);

        DebugLvl1("\\SaveToFile");
    }*/

    // --------------------------------------------------------------------------------

    QString UdpLogDB::GetCopy(int start, int count)
    {
        DebugLvl1("GetCopy");

        QString res;

        if (count < 0) count = RecordCount();

        // fix: 'start' is sent from other thead and DB can be dropped before we received it
        if (start > RecordCount()) start = 0;
        if (count > (RecordCount() - start)) count = RecordCount();

        // count = 0 -> use dbSize as count
        if (count == 0) count = RecordCount();


        ToString(start, count, res);

        DebugLvl1("\\GetCopy");

        return res;
    }

    // --------------------------------------------------------------------------------

    void UdpLogDB::ToString(int start, int count, QString &res) const
    {
        res.clear();

        ESS_ASSERT((start >= 0) && (count >= 0));
        if (m_db.Size() == 0) return;

        ESS_ASSERT(start < m_db.Size());
        ESS_ASSERT((start + count) <= m_db.Size());

        size_t end = start + count;
        for (size_t i = start; i < end; ++i)
        {
            res += m_db.Get(i, m_recViewSett, m_settVersion).Get();
            if (i < (end - 1)) res += "\n";
        }
    }

    // --------------------------------------------------------------------------------

    void UdpLogDB::ForceRefilter()
    {
        DebugLvl1("ForceRefilter");

        m_db.Refilter(m_recViewSett);

        DebugLvl1("\\ForceRefilter");
    }

    // --------------------------------------------------------------------------------

    void UdpLogDB::DebugLvl1(const QString &text) const
    {
        if (m_debugLevel != 1) return;

        QString msg = QString("Debug [UdpLogDB]: %1").arg(text);
        std::cout << msg.toStdString() << std::endl;
    }

    // --------------------------------------------------------------------------------

    void UdpLogDB::UpdateStatus(ddword refreshMsgCount,
                                ddword renderImgCount, ddword renderImgAvgTimeMs)
    {
        m_status.RefreshMsgCount(refreshMsgCount);
        m_status.Rendering(renderImgCount, renderImgAvgTimeMs);
    }

    // --------------------------------------------------------------------------------

} // namespace Ulv
