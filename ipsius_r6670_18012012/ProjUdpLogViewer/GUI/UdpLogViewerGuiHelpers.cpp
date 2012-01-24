#include "UdpLogViewerGuiHelpers.h"

#include "stdafx.h"
#include "Tests/DebugSrcList.h"

namespace
{
    using namespace Ulv;

    const QString CIconsPath = "Images/";

    // ----------------------------------------------------------------

    const QString CTimestampFormatLong = "dd.MM.yyyy hh:mm:ss";
    const QString CTimestampFormatShort = "hh:mm:ss.zzz";

    // --------------------------------------------------------------

    QString FormatSize(ddword size)
    {
        QString res("%1 %2b");
        ddword kb = 1024;
        ddword mb = kb * kb;
        ddword gb = mb * kb;

        if (size >= gb) return res.arg(size / gb).arg("G");
        if (size >= mb) return res.arg(size / mb).arg("M");
        if (size >= kb) return res.arg(size / kb).arg("K");

        return res.arg(size).arg("");
    }


} // namespace

// ----------------------------------------------------------------

namespace UlvGui
{
    using namespace UlvTests;
    using namespace Ulv;


    // ----------------------------------------------------------------

    void ShowErrorImpl(QWidget *pOwner, const QString &err)
    {
        QErrorMessage e(pOwner);
        e.setModal(true);
        e.showMessage(err);
        e.exec();
    }

    // --------------------------------------------------------------

    bool CheckBoxStateToBool(const QCheckBox &box)
    {
        return (box.checkState() == Qt::Unchecked)? false : true;
    }

    // --------------------------------------------------------------

    QString PortTypeToString(UdpPortDataType type)
    {
        switch (type)
        {
        case PTString:
            return QString("ASCII");

        case PTWstring:
            return QString("UNICODE");

        default:
            ESS_HALT("Invalid port data type");
        }

        return QString();
    }

    // --------------------------------------------------------------

    QString IconPath(const QString &fileName)
    {
        QString fullPath = ":/" + CIconsPath + fileName;

        return fullPath;
    }

    // --------------------------------------------------------------

    bool TimeElapsed(const QDateTime &startDT, dword mSec)
    {
        QDateTime expected(startDT);
        expected = expected.addMSecs(mSec);

        return (QDateTime::currentDateTime() >= expected);
    }

    // --------------------------------------------------------------

    Ulv::TimestampView MakeTimestampView(const QComboBox &timestampBox)
    {
        QString desc = timestampBox.currentText();

        bool dontDisplayTimestamp = desc.contains("none", Qt::CaseInsensitive);
        if (dontDisplayTimestamp) return Ulv::TimestampView();

        Ulv::TimestampView::TspType type = desc.contains("sender", Qt::CaseInsensitive)
                                           ? Ulv::TimestampView::SenderSide
                                           : Ulv::TimestampView::ReceiverSide;
        QString format = desc.contains("short")? CTimestampFormatShort : CTimestampFormatLong;

        return Ulv::TimestampView(type, format);
    }

    // --------------------------------------------------------------

    QString PacketsStatus(ddword all, ddword bytes, ddword invalid, dword inDB, ddword dropped,
                          dword filtered, bool showFiltered)
    {
        QString res =
                QString("[ Packets: %2; %3 Kb (all); %4 (invalid); %5 (in DB); %6 (dropped)")
                            .arg(all).arg(bytes / 1024).arg(invalid).arg(inDB).arg(dropped);
        if (showFiltered) res += QString("; %1 (filtered)").arg(filtered);
        res += " ]";

        return res;
    }

    // --------------------------------------------------------------

    QString ErrorsStatus(ddword count)
    {
        return QString("[ Errors: %2 ]").arg(count);
    }

    // --------------------------------------------------------------

    QString DebugStatus(dword activeCount, dword allCount)
    {
        return QString("[ Debug sources(active): %1(%2) ]").arg(allCount).arg(activeCount);
    }

    // --------------------------------------------------------------

    QString ScrollStatus(const QScrollBar &scroll)
    {
        QString res = QString("[ Scroll: %2 (pos); %3 (size); %4 (page step) ]")
                              .arg(scroll.value()).arg(scroll.maximum())
                              .arg(scroll.pageStep());
        return res;
    }

    // --------------------------------------------------------------

    QString TimeStatus(const QDateTime &startTime)
    {
        QDateTime curr = QDateTime::currentDateTime();

        int days = startTime.daysTo(curr);
        int secs = startTime.time().secsTo(curr.time());

        if ((days > 0) && (curr.time() < startTime.time())) --days;

        QTime time = QTime().addSecs(secs);

        QString res = QString("[ Running: %1d %2h %3m %4s ]")
                      .arg(days).arg(time.hour()).arg(time.minute()).arg(time.second());
        return res;
    }

    // --------------------------------------------------------------

    QString RenderStatus(ddword imgRendered, ddword imgRenderingAvgTime, ddword imgPainted,
                         ddword refreshMsgAll, ddword refreshMsgProcessed)
    {
        QString res = QString("[ Images: %1 / 1 per %5 ms (rendered); %2 (painted); "
                              "Refresh: %3 / %4 (all / processed) ]")
                              .arg(imgRendered).arg(imgPainted)
                              .arg(refreshMsgAll).arg(refreshMsgProcessed)
                              .arg(imgRenderingAvgTime);
        return res;
    }

    // --------------------------------------------------------------

    QString DropStatus(ddword times)
    {
        return QString("[ DB drops: %1 ]").arg(times);
    }

    // --------------------------------------------------------------

    QString RecStatus(ddword times, ddword fileSize)
    {
        QString res = QString("[ Rec: %1 (times), %2 (file size) ]")
                      .arg(times).arg(FormatSize(fileSize));

        return res;
    }


} // namespace UlvGui
