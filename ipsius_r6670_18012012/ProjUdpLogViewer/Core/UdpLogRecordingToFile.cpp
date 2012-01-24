
#include "stdafx.h"

#include "UdpLogRecordingToFile.h"
#include "UlvUtils.h"


namespace
{
    const QString CLogDir = "Log/";
    const QString CLogFilePref = "AutoLog";
    const QString CLogFileExt = "txt";

} // namespace

// ------------------------------------------------------------------------------

namespace Ulv
{
    UdpLogRecordingToFile::UdpLogRecordingToFile()
    {
        QString err;
        m_file = UlvUtils::GenAbsFileName(CLogDir, CLogFilePref, CLogFileExt, err);
        if (!err.isEmpty()) ESS_THROW_MSG(Ulv::RecordingErr, err.toStdString());

        TimestampView ts(TimestampView::ReceiverSide, "< dd.MM.yyyy hh:mm:ss >");
        m_format.UsedTimestamp(ts);
        m_format.UseRecordSrcInView(true);
    }

    // ------------------------------------------------------------------------------
    // IUdpLogRuntimeRecording impl

    void UdpLogRecordingToFile::Add(const QString &formattedData)
    {
        bool append = QFile::exists(m_file);
        QString err;

        UlvUtils::SaveToFile(m_file, formattedData, append, err);

        if (err.isEmpty()) return;

        QString msg = QString("DB recording error: %1").arg(err);
        ESS_THROW_MSG(Ulv::RecordingErr, msg.toStdString());
    }

    // -----------------------------------------------------------

    QString UdpLogRecordingToFile::DestDescription() const
    {
        return QString("file '%1'").arg(m_file);
    }

    // -----------------------------------------------------------

    ddword UdpLogRecordingToFile::RecordedSize() const
    {
        QFile f(m_file);
        return f.size();
    }

} // namespace Ulv
