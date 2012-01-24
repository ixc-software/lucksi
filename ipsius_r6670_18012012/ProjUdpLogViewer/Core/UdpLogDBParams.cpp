
#include "stdafx.h"

#include "UdpLogDBParams.h"
#include "UdpPackParser.h"
#include "UdpLogRecordingToFile.h"
#include "UlvUtils.h"

namespace
{
    const int CDefDBRecordsCount = 10000;
    const int CDefDBDropRecordsCount = CDefDBRecordsCount / 2;
    const int CDefRecordingIntervalMs = /*30 * 1000;*/ 10 * 60 * 1000;
    const int CDefRecordingRecordCount = 500;


} // namespace

// -------------------------------------------------------------------

namespace Ulv
{
    UdpLogDBParams::UdpLogDBParams() :
        m_recordsCount(CDefDBRecordsCount),
        m_dropRecordsCount(CDefDBDropRecordsCount),
        m_parser(new Ulv::UdpPackParser),
        m_recorder(new UdpLogRecordingToFile),
        m_recIntervalMs(CDefRecordingIntervalMs),
        m_recRecordCount(CDefRecordingRecordCount)
    {
    }

    // -------------------------------------------------------------------

    void UdpLogDBParams::RecordCount(int count)
    {
        ESS_ASSERT(count > 0);
        m_recordsCount = count;
        m_dropRecordsCount = count / 2;
    }

    // -------------------------------------------------------------------

    bool UdpLogDBParams::RecordingEnabled() const
    {
        return (m_recorder != 0);
    }

    // -------------------------------------------------------------------

    void UdpLogDBParams::EnableRecording(bool state)
    {
        if (state) return;

        m_recorder.reset();
    }

    // -------------------------------------------------------------------

    QString UdpLogDBParams::RecordingTo() const
    {
        ESS_ASSERT(m_recorder != 0);
        return m_recorder->DestDescription();
    }

    // -------------------------------------------------------------------

    IUdpLogRuntimeRecording& UdpLogDBParams::Recorder()
    {
        ESS_ASSERT(m_recorder != 0);

        return *m_recorder.get();
    }

    // -------------------------------------------------------------------

    IUdpPackParser& UdpLogDBParams::Parser()
    {
        ESS_ASSERT(m_parser != 0);

        return *m_parser.get();
    }

    // -------------------------------------------------------------------

    const IUdpPackParser& UdpLogDBParams::Parser() const
    {
        ESS_ASSERT(m_parser != 0);

        return *m_parser.get();
    }

    // -------------------------------------------------------------------

    int UdpLogDBParams::DefaultRecordCount()
    {
        return CDefDBRecordsCount;
    }

    // -------------------------------------------------------------------

    int UdpLogDBParams::DefaultRecordingIntervalMs()
    {
        return CDefRecordingIntervalMs;
    }

    // -------------------------------------------------------------------

    int UdpLogDBParams::DefaultRecordingRecordCount()
    {
        return CDefRecordingRecordCount;
    }

} // namespace Ulv
