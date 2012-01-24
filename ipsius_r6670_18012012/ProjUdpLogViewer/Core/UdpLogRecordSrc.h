#ifndef __UDPLOGRECORDSRC__
#define __UDPLOGRECORDSRC__

#include "UlvTypes.h"

#include "Utils/HostInf.h"
#include "Utils/SafeRef.h"
#include "Utils/ManagedList.h"


namespace Ulv
{
    struct UdpLogRecordSrcInfo // copyable
    {
        QString UniqueName;
        bool Enabled;

        dword Packets;
        dword Bytes;

        UdpLogRecordSrcInfo(const QString &name, bool enabled) :
                UniqueName(name), Enabled(enabled), Packets(0), Bytes(0)
        {}

        QString ToString() const;
    };

    // ------------------------------------------------------------------

    // UDP packet sender's info
    class UdpLogRecordSrc :
            public virtual Utils::SafeRefServer,
            boost::noncopyable
    {
        Utils::HostInf m_host;

        UdpLogRecordSrcInfo m_info;

    public:
        UdpLogRecordSrc(Utils::HostInf host, bool enabled);

        void AddInfo(dword newReceivedBytes);

        void Enable(bool state) { m_info.Enabled = state; }
        bool Enabled() const { return m_info.Enabled; }

        const QString& UniqueName() const { return m_info.UniqueName; }
        dword PacketsSent() const { return m_info.Packets; }
        dword BytesSent() const { return m_info.Bytes; }

        const UdpLogRecordSrcInfo& Info() const { return m_info; }

        bool operator==(const QString &name) const;
        bool operator!=(const QString &name) const;

        bool operator==(const Utils::HostInf &host) const;
        bool operator!=(const Utils::HostInf &host) const;        
    };

    // ---------------------------------------

    class UdpLogRecordSrcInfoList // copyable
    {
        std::vector<UdpLogRecordSrcInfo> m_list;

        int Find(const QString &name) const;

    public:
        UdpLogRecordSrcInfoList()
        {}

        void Add(const UdpLogRecordSrcInfo &item);

        void Enable(int i, bool state);
        void Enable(const QString &name, bool state);

        bool Enabled(int i) const;
        bool Enabled(const QString& name) const;

        void Clear() { m_list.clear(); }

        const UdpLogRecordSrcInfo& Get(int index) const;
        int Size() const { return m_list.size(); }        
    };

    // ---------------------------------------

    // List of record sources
    class UdpLogRecordSrcList :
            boost::noncopyable
    {
        Utils::ManagedList<UdpLogRecordSrc> m_list;

        int Find(const QString &name) const;

    public:
        UdpLogRecordSrcList() : m_list(true)
        {}

        Utils::SafeRef<UdpLogRecordSrc> AddOrUpdate(const Utils::HostInf &host,
                                                    dword packSize,
                                                    bool enabled = true);
        void Clear() { m_list.Clear(); }

        // return true if selection was changed
        bool SyncSelection(const UdpLogRecordSrcInfoList &other);

        Utils::SafeRef<UdpLogRecordSrc> Get(size_t index) const;

        size_t Size() const { return m_list.Size(); }

        UdpLogRecordSrcInfoList InfoList() const;
    };

} // namespace Ulv

#endif
