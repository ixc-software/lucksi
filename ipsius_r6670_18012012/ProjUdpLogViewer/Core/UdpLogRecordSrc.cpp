
#include "stdafx.h"

#include "UdpLogRecordSrc.h"


namespace Ulv
{
    // ----------------------------------------------------------
    // UdpLogRecordSrcInfo impl

    QString UdpLogRecordSrcInfo::ToString() const
    {
        QString res(UniqueName);
        res += ": ";
        res += QString::number(Packets);
        res += " / ";
        res += QString::number(Bytes / 1024);
        res += "Kb";

        return res;
    }

    // ----------------------------------------------------------
    // UdpLogRecordSrc impl

    UdpLogRecordSrc::UdpLogRecordSrc(Utils::HostInf host, bool enabled) :
        m_host(host), m_info(m_host.ToString().c_str(), enabled)
    {
    }

    // ----------------------------------------------------------

    void UdpLogRecordSrc::AddInfo(dword newReceivedBytes)
    {
        ++m_info.Packets;
        m_info.Bytes += newReceivedBytes;
    }

    // ----------------------------------------------------------

    bool UdpLogRecordSrc::operator==(const QString &name) const
    {
        return (this->m_info.UniqueName == name);
    }

    // ----------------------------------------------------------

    bool UdpLogRecordSrc::operator!=(const QString &name) const
    {
        return !operator==(name);
    }

    // ----------------------------------------------------------

    bool UdpLogRecordSrc::operator==(const Utils::HostInf &host) const
    {
        return (this->m_host == host);
    }

    // ----------------------------------------------------------

    bool UdpLogRecordSrc::operator!=(const Utils::HostInf &host) const
    {
        return !operator==(host);
    }

    // ----------------------------------------------------------
    // UdpLogRecordSrcInfoList impl

    int UdpLogRecordSrcInfoList::Find(const QString &name) const
    {
        for (int i = 0; i < m_list.size(); ++i)
        {
            if (m_list.at(i).UniqueName == name) return i;
        }

        return -1;
    }

    // ----------------------------------------------------------


    void UdpLogRecordSrcInfoList::Add(const UdpLogRecordSrcInfo &item)
    {
        m_list.push_back(item);
    }

    // ----------------------------------------------------------

    const UdpLogRecordSrcInfo& UdpLogRecordSrcInfoList::Get(int index) const
    {
        ESS_ASSERT(index < Size());
        return m_list.at(index);
    }

    // ----------------------------------------------------------

    void UdpLogRecordSrcInfoList::Enable(int i, bool state)
    {
        ESS_ASSERT(i < Size());
        m_list[i].Enabled = state;
    }

    // ----------------------------------------------------------

    void UdpLogRecordSrcInfoList::Enable(const QString &name, bool state)
    {
        ESS_ASSERT(Size() != 0);

        int i = Find(name);
        if (i >= 0)
        {
            Enable(i, state);
            return;
        }

        ESS_HALT("Debug: unknown source name");
    }

    // ----------------------------------------------------------

    bool UdpLogRecordSrcInfoList::Enabled(const QString& name) const
    {
        ESS_ASSERT(Size() != 0);

        int i = Find(name);
        if (i >= 0) return Enabled(i);

        ESS_HALT("Debug: unknown source name");

        return false; // dummi
    }

    // ----------------------------------------------------------

    bool UdpLogRecordSrcInfoList::Enabled(int i) const
    {
        ESS_ASSERT(i < Size());
        return m_list[i].Enabled;

    }

    // ----------------------------------------------------------
    // UdpLogRecordSrcList impl

    Utils::SafeRef<UdpLogRecordSrc> UdpLogRecordSrcList::AddOrUpdate(const Utils::HostInf &host,
                                                                     dword packSize,
                                                                     bool enabled)
    {
        for (size_t i = 0; i < Size(); ++i)
        {
            if (*m_list[i] == host)
            {
                // update
                m_list[i]->AddInfo(packSize);
                return m_list[i];
            }
        }

        // add
        UdpLogRecordSrc *pItem = new UdpLogRecordSrc(host, enabled);
        pItem->AddInfo(packSize);
        m_list.Add(pItem);

        return m_list.Back();
    }

    // ----------------------------------------------------------

    bool UdpLogRecordSrcList::SyncSelection(const UdpLogRecordSrcInfoList &other)
    {
        if (Size() == 0) return false;

        bool wasChanged = false;

        // can be diff order
        for (size_t i = 0; i < other.Size(); ++i)
        {
            // Utils::SafeRef<UdpLogRecordSrc> selfItem = Get(other.Get(i).UniqueName);

            int selfItemIndex = Find(other.Get(i).UniqueName);
            if (selfItemIndex < 0) continue; // GUI have outdated src

            Utils::SafeRef<UdpLogRecordSrc> selfItem = Get(selfItemIndex);

            if (selfItem->Enabled() == other.Get(i).Enabled) continue;

            selfItem->Enable(other.Get(i).Enabled);
            wasChanged = true;
        }

        return wasChanged;
    }

    // ----------------------------------------------------------

    Utils::SafeRef<UdpLogRecordSrc> UdpLogRecordSrcList::Get(size_t index) const
    {
        ESS_ASSERT(index < Size());

        return m_list[index];
    }

    // ----------------------------------------------------------

   /* Utils::SafeRef<UdpLogRecordSrc> UdpLogRecordSrcList::Get(const QString &name) const
    {
        ESS_ASSERT(Size() > 0);

        for (size_t i = 0; i < Size(); ++i)
        {
            if (*m_list[i] == name) return m_list[i];
        }

        ESS_HALT(QString("Debug: Unknown log record source: '%1'!").arg(name).toStdString());

        return m_list[0]; // dummi
    }*/

    int UdpLogRecordSrcList::Find(const QString &name) const
    {
        for (size_t i = 0; i < Size(); ++i)
        {
            if (*m_list[i] == name) return i;
        }

        return -1;
    }

    // ----------------------------------------------------------

    UdpLogRecordSrcInfoList UdpLogRecordSrcList::InfoList() const
    {
        UdpLogRecordSrcInfoList res;

        for (size_t i = 0; i < Size(); ++i)
        {
            res.Add(Get(i)->Info());
        }

        return res;
    }

} // namespace Ulv
